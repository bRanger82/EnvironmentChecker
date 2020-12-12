#ifndef DISPLAY_HEADER

#define DISPLAY_HEADER
#include <Adafruit_Sensor.h>
#include <Adafruit_SSD1306.h>

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET    -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool InitDisplay(void)
{
  bool state = false;  // Use the pessimistic approach, initialization is not successful
  byte i2c_adresses[] = {0x3C, 0x3D}; // Can be either I2C OLED display adresses

  // Try each I2C adresses if available
  for (byte idx = 0; idx <= 1; idx++)
  {
    if (display.begin(SSD1306_SWITCHCAPVCC, i2c_adresses[idx]))
    {
      state = true; // OLED display found :)
      break;
    }
  }

  if (state)
  {
    display.clearDisplay();  
    display.setTextSize(1);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);                 // Use full 256 char 'Code Page 437' font
    display.display();    
  }

  return state;
}

#endif
