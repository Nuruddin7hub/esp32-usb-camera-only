/* Minimal USB camera test for a generic ESP32-S3 board.
 * Based on Espressif esp-iot-solution usb_camera_lcd_display example,
 * with ALL display / BSP board code removed.
 * It does NOT decode JPEG, it does NOT touch any LCD.
 * It just logs: camera connected, frame size, resolution, frame count.
 *
 * HEADLESS MODE (for boards where PROG + CAM USB plugs cannot fit
 * at the same time): an external LED on GPIO2 shows the result
 * without a monitor:
 *   LED off        = waiting for camera
 *   LED on solid   = camera connected, streaming starting
 *   LED blinking   = frames are coming in (about 2x per second)
 *   LED off again  = camera disconnected
 * Wire: GPIO2 -> 220 ohm resistor -> LED long leg (anode),
 *       LED short leg (cathode) -> GND.
 * Power the ESP32 from the 5V pin + GND in headless mode, with the
 * PROG USB cable UNPLUGGED (do not feed 5V from two sources at once).
 * Plug the USB camera into the ESP32-S3 native USB (OTG) port AFTER
 * flashing, while the board is powered.
 */
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "app_uvc.h"

#define LED_GPIO GPIO_NUM_2

static const char *TAG = "usb_camera_only";
static volatile bool camera_connected = false;
static uint32_t frame_count = 0;

static void camera_frame_cb(const uvc_host_frame_t *frame, void *user_ctx)
{
    (void)user_ctx;
    if (!camera_connected) {
        return;
    }
    frame_count++;
    /* Log every frame for the first 5, then every 30th, so the monitor
     * is readable and still proves frames keep coming. */
    if (frame_count <= 5 || (frame_count % 30) == 0) {
        ESP_LOGI(TAG, "FRAME %lu: %u bytes, %ux%u, format=%d",
                 (unsigned long)frame_count,
                 (unsigned int)frame->data_len,
                 (unsigned int)frame->vs_format.h_res,
                 (unsigned int)frame->vs_format.v_res,
                 (int)frame->vs_format.format);
    }
}

static void camera_status_cb(bool connected, void *user_ctx)
{
    (void)user_ctx;
    camera_connected = connected;
    frame_count = 0;
    if (connected) {
        ESP_LOGI(TAG, "Camera connected, ready for frames");
        gpio_set_level(LED_GPIO, 1);
    } else {
        ESP_LOGW(TAG, "Camera disconnected");
        gpio_set_level(LED_GPIO, 0);
    }
}

void app_main(void)
{
    /* LED off = waiting. Set up before anything else. */
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&led_cfg);
    gpio_set_level(LED_GPIO, 0);

    /* Boot blink: 3 quick blinks prove the code is running AND that
     * the LED is really on GPIO2. If you do NOT see these 3 blinks
     * on power-up, the LED wire is on the wrong pin — fix that first,
     * before worrying about the camera. After the blinks, LED goes
     * off = waiting for camera. */
    for (int i = 0; i < 3; i++) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    ESP_LOGI(TAG, "USB camera only test started - no LCD, no BSP board code");
    ESP_LOGI(TAG, "LED on GPIO2: off=waiting, on=connected, blink=streaming");
    ESP_LOGI(TAG, "Waiting for camera... plug USB camera into the native USB (OTG) port now");

    app_uvc_config_t uvc_config = {
        .max_width = 800,
        .max_height = 480,
        .frame_cb = camera_frame_cb,
        .status_cb = camera_status_cb,
        .user_ctx = NULL,
    };
    ESP_ERROR_CHECK(app_uvc_init(&uvc_config));

    /* 0x0 preferred = use the camera's default resolution.
     * app_uvc_start blocks until a camera is connected. */
    app_uvc_frame_size_t preferred = {0, 0};
    app_uvc_frame_size_t current = {0, 0};
    ESP_ERROR_CHECK(app_uvc_start(&preferred, &current));
    ESP_LOGI(TAG, "Streaming started at %ux%u", current.width, current.height);

    /* Blink while connected, off if disconnected. This is the
     * headless indicator: no monitor needed to see it work. */
    while (1) {
        if (camera_connected) {
            gpio_set_level(LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(250));
            gpio_set_level(LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(250));
        } else {
            gpio_set_level(LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}
