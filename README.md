# ISS-Tracker
Assembly instructions available here: https://www.printables.com/model/383268-iss-tracker

The electronics here are based around an Adafruit Feather m0 Express with a Featherwing ESP32 Wifi coprocessor. In order to build with the arduino IDE, you'll need to install Adafruit boards and SAMD support. Follow the guide here: https://learn.adafruit.com/adafruit-feather-m0-express-designed-for-circuit-python-circuitpython/arduino-ide-setup

In addition to the other dependencies, which can be installed with the standard Arduino IDE library manager, the Airlift Featherwing uses an Adafruit fork of the WiFiNINA library. See here for additional details: https://learn.adafruit.com/adafruit-airlift-featherwing-esp32-wifi-co-processor-featherwing/arduino

## Configuration
You'll need to make some minor modifications before getting up and running. The biggest ones are found in arduino_secrets.h, where you'll need to fill in your wifi SSID and password so that it can actually connect to the internet, and your precise latitude & longitude so that it can perform proper pointing. If you end up forking this code, make absolutely sure not to push any commits with these fields filled in.

Aside from that, all other configuration parameters can be found in defs.h. They should all hopefully be pretty self-explanatory. If you are omitting the compass, make sure you set DO_BYPASS_COMPASS to true. Also be sure to set your timeZone properly so that the displayed time is correct. Finally, you may need to adjust the SERVO_MIN_PWM and SERVO_MAX_PWM parameters so that the pointer properly points with the correct elevation. On startup, the servo will run through a test cycle, attempting to point at 0 degrees, then -90, then +90, then back to 0. This should give you a chance to make sure that the PWM range specified by these parameters is correct. If it isn't, double-check the spec sheet for your micro-servo of choice.

## Notes on electical connections
A simple schematic is included under Schematic.png. Note that the compass and OLED Featherwing are both connected using 4-wire stemma QT cables. Note also that the Wifi featherwing is not included in the schematic as it just stacks directly on top of the Feather M0 using stacking headers. In my build I used a perma-proto board to handle both distribution of power and breaking out the I2C clock and data signals to both the compass and display.

## Dependencies:
Adafruit GFX Library

Adafruit SH110X

Adafruit MMC56x3

AccelStepper

TimeLib

WiFiNINA (Adafruit fork, see above)
