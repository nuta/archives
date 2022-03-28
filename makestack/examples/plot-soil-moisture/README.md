# Plot soil moisture (or how to integrate with Thingspeak)

## How To
1. Create a channel on ThingSpeak and update `THINGSPEAK_API_KEY` with the generated *Write API Key*.
2. Run `makestack deploy` to deploy the server-side on Firebase.
3. Run `makestack flash` to install the firmware on your device.
4. Connect with the sensor on the pin specified in `pin` and turn on the device.
