# LEDROCK

LEDROCK is a custom controller for the IKEA LEDBERG RGB LED strip, built around an ESP32.

The project replaces the original LEDBERG controller and adds multiple ways of controlling the strip. It can be controlled over Wi-Fi using WebSockets, or directly over a serial connection using a custom command protocol.

A desktop light-bar application can capture the colors along the lower edge of the PC display and reproduce them on the LED strip, turning it into a lightweight screen-reactive ambient lighting system.

The repository contains the complete project, including:

- ESP32 firmware - LED control, WebSocket interface, and serial command protocol
- Custom controller hardware
- Desktop light-bar application - mirrors colors from the lower portion of the display
- Command-line application - direct control of the LED strip over USB-C

The project combines custom hardware, embedded firmware, networking, serial communication, and desktop software into a complete LED control system.
