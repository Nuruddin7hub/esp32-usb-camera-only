# USB camera only test — generic ESP32-S3, no LCD board

Stripped from Espressif esp-iot-solution `usb_camera_lcd_display`.
Display / BSP board code removed, so it cannot crash on a missing
onboard LCD. It only logs camera frames, and blinks an LED on GPIO2
so it can be tested without a monitor.

## Build and flash (in ESP-IDF PowerShell, ESP32 plugged into PROG/COM11)

```
idf.py set-target esp32s3
idf.py build
idf.py -p COM11 flash
```

Flash with only the PROG cable connected. No camera yet.

## LED wiring (do this before the headless test)

On a breadboard:
- ESP32 GPIO2 -> 220 ohm resistor -> LED long leg (anode)
- LED short leg (cathode) -> GND

LED meaning:
- Off = waiting for camera
- On solid = camera connected
- Blinking (about 2x per second) = frames are coming in
- Off again = camera disconnected

## Headless test with the breadboard power module

This is for boards where the PROG USB cable and the camera USB
adapter cannot fit in at the same time.

1. Flash the board (PROG cable only, step above), then unplug PROG.
2. Set the breadboard power module jumper to 5V on the rail you use.
   Power the module with its barrel jack adapter (7-12V).
   Its USB-A port is a 5V OUTPUT, not an input — do not try to
   power the module from USB.
3. Power the ESP32 from the breadboard, NOT from USB at the same time:
   - Breadboard 5V rail -> ESP32 5V / 5Vin pin
   - Breadboard GND rail -> ESP32 GND pin
   - PROG USB cable stays UNPLUGGED while the 5V pin is powered.
4. Plug the USB camera into the ESP32-S3 native USB (OTG) port —
   the CAM port, not COM11.
5. Watch the LED:
   - Off at first, then on solid, then blinking = camera works.
   - Still off after 10 seconds = camera not detected / not powered.
     Check MJPEG format, USB 1.1 full-speed, max 800x480 in this test.

If you CAN fit both USB plugs, you can also just run
`idf.py -p COM11 flash monitor` and watch for:
- "UVC device connected"
- "Camera connected, ready for frames"
- "Streaming started at WxH"
- "FRAME 1: N bytes, WxH, format=..."
- Then FRAME 30, 60, 90, ...

If no camera is detected, check:
- Camera needs MJPEG output, USB 1.1 full-speed, max 800x480 in this test.
- Non-MJPEG frames are dropped by app_uvc.c with
  "Drop unsupported UVC frame format".

To exit the monitor (monitor mode only): Ctrl + ]
