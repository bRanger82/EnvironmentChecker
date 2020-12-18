# EnvironmentChecker

Check out the PCB Design: https://oshwlab.com/michi/environmentchecker

# Motivation

The main motivation factor for this project was to have the flexibility compared to buyed products. 
Most of them do not met my requirements for the product or, if they do, they are rather expensive. 

The main reasons:
- most of the buy options do no have the ability for an manual update (you have to wait minutes until the temperature reading is updated)
- most of the buy options have an unknown temperature sensor on it. For me as a tinkerer it is always nice to know which parts are included and what are they capable of 
- honestly, as a tinkerer is it always nice to create something and have some kind of "product individuality" in the self created product

# Notes

Please note that the PCB design and the code are created based on other code parts (like Adafruit GFX library). So the project is not built up from scratch.
The reason for this is simple, it is a small project created over night. In the future this project could be improved to have it built up from scratch, but for now this is out-of-scope. 

# PCB design decisions

In general, the PCB consists of 3 parts:
- Source power part: the fuse should prevent an overcurrent, the MOSFET and its releated parts are used for reverse voltage protection. Also a Buck-Converter is used instead of the linare voltage regulators due to power efficiency reasons.
The main reason was to prevent me for doing something bad, e.g. reverse voltage which might destroy the entire circuit. A simple diode may have done the job as well, but the efficiency factor was more important for me than the cost (money and space on the PCB).

- Processing part: this part contains the ATMEGA328P-AU (please note: this is __not__ the ATMEGA328P~~B~~-AU one!) processor, the two I2C components (OLED display and the BME280 sensor) and the LEDs showing the state of the air measurement. 
Reason for the BME280 sensor: it is a very reliant environment sensor which is easy to read using the I2C interface.
Reason for the OLED display: the 0,96inch variant was used for this project, as it provides enough space to display the sensor data and has a compact (and rather small) form factor.

- User input part: until now, only two buttons are available: __Display__ and __Setup__. While the Display button is showing the current BME280 reading on the display, the Setup button is currently not used. 
What the Display button does: it shows the BME280 sensor values for a short period of time on the OLED display. After a timeout exceeds, the display is turned off again. The main reason for that is the power consumption. While the display shows the data and the status LED lights up, the power consumption reaches up to 20 mA. 
When the display and status LED are turned off after a defined period of time, the power consumption will be lowered to about 3 mA. 

# Disclaimer 
The entire project (the code, the schematic/PCB design and all related parts) is just for educational use. Using the code and/or schematic/PCB on your own risk. 
