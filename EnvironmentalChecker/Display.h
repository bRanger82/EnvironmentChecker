#ifndef DISPLAY_HEADER

#define DISPLAY_HEADER
#include <Adafruit_SSD1306.h>

#define I2C_ADDR_OLED_DISPLAY 0x3C

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET    -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool InitDisplay(void)
{
  bool state = false;  // Use the pessimistic approach, initialization is not successful

  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    state = true; // OLED display found :)
  }

  if (state)
  {
    display.clearDisplay();  
    display.setTextSize(1);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(false);                 // Use full 256 char 'Code Page 437' font
    display.display();    
  }

  return state;
}

#endif
