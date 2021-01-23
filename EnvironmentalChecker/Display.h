#ifndef DISPLAY_HEADER

#define DISPLAY_HEADER
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "DejaVu_Serif_14.h"
#define I2C_ADDR_OLED_DISPLAY 0x3C

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET    -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool InitDisplay(void)
{
  bool state = false;  // Use the pessimistic approach, initialization is not successful

  if (display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR_OLED_DISPLAY))
  {
    state = true; // OLED display found :)
  }

  if (state)
  {
    display.clearDisplay();  
    display.setFont(&DejaVu_Serif_14);
    display.setTextSize(1);              // Normal 1:1 pixel scale
    
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);                 // Use full 256 char 'Code Page 437' font
    display.drawRect(2, 2, 124, 60, WHITE);
    display.setCursor(8, 25);            // Start at top-left corner
    display.println(F("Startup ... "));
    display.setCursor(8, 50); 
    display.println(F("Please wait!"));
    display.display();    
  }

  return state;
}

void PrintDataOnDisplay(float Temperature, float Pressure, float Humidity, float DewPoint, bool TempPart)
{
  display.clearDisplay(); 
  display.drawRect(2, 2, 124, 60, WHITE);
  if (TempPart)
  {
    display.setCursor(8, 25);
    display.print(F("T: "));
    display.print(Temperature);
    display.print(F(" *"));
    display.println(F("C"));    

    display.setCursor(8, 51);
    display.print(F("H: "));
    display.print(Humidity);
    display.println(F(" %"));
  } else
  {
    display.setCursor(8, 25);
    display.print(F("P: "));
    display.print(Pressure);
    display.println(F(" hPa"));
    
    display.setCursor(8, 51);
    display.print(F("DP: "));
    display.print(DewPoint);
    display.print(F(" *"));
    display.println(F("C")); 
  }

  display.display();
}
#endif
