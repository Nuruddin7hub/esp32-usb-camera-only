# esp32-usb-camera-only

ESP32-S3 acting as a **USB host for a UVC camera** (camera -> ESP32 direction),
built with ESP-IDF. Streams MJPEG from a USB webcam over the ESP32-S3's native
USB OTG port (GPIO19/20) with LED status indication.

> Status: code complete, waiting on hardware (powered USB hub / JST cable).

## Build

Standard ESP-IDF v5.x flow:

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p <PORT> flash monitor
```
