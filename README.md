# Meteorology
MiP tells the weather. I'm pretty proud of this Arduino sketch. It is, in part, what drove me to update the [MPU-D1 mini library](https://github.com/Tiogaplanet/MPU_D1_mini_lib).  This sketch, along with the MPU-D1 mini library and ThingPulse's [esp8266-weather-station](https://github.com/ThingPulse/esp8266-weather-station) library, transform MiP into a meteorologist.  Just like in *The Matrix*, when Neo gets his jujitsu upload, MiP instantly knows the weather and can tell you about it using its eye LEDs, chest LED, and it hosts a webpage so you can look it up on your computer or phone.

The color of MiP's chest LED changes with the temperature.  The eyes indicate rain, randomly blinking to show the rain's intensity.  The web page MiP hosts, accessible using MiP's IP address, shows complete weather data for the location of your choice.

<img width="349" height="557" alt="meteorology" src="https://github.com/user-attachments/assets/a5a924e1-8ae1-4922-abc1-78131979b2f2" />

The page background indicates time of day, and MiP's chest LED is reproduced for easy viewing.

What will MiP do next?