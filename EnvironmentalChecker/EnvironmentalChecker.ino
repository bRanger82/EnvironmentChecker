#include <Wire.h>
#include "BME.h"
#include "Display.h"

volatile bool btn_setup_pressed = false;
volatile bool btn_display_pressed = false;

#define BUTTON_DISPLAY 2
#define BUTTON_SETUP   3
#define LED_HIGH_CRIT  5
#define LED_HIGH_WARN  6
#define LED_NORMAL     7
#define LED_LOW_WARN   8
#define LED_LOW_CRIT   9

int DisplayTimeout = 0;   // counter increased each run in loop -> when threshold is reached, the display is turned off
int SensorTimeout  = 0;   // counter increased each run in loop -> when threshold is reached, the BME280 sensor values are read

void PrintValuesDisplay(void)
{
  display.setCursor(0, 0);             // Start at top-left corner
  display.print(F("Temp: "));
  display.print(Temperature);
  display.println(F(" C"));
  display.println(F(" "));
  display.print(F("Pressure: "));
  display.print(Pressure);
  display.println(F(" hPa"));
  display.println(F(" "));
  display.print(F("Humidity: "));
  display.print(Humidity);
  display.println(F(" %"));
  display.println(F(" "));
  display.print(F("Dew Point: "));
  display.print(DewPoint);
  display.println(F(" C"));
  
  display.display();

  DisplayTimeout = 0;
}

void getSensorValues(void)
{
  Temperature = bme.readTemperature();
  Pressure = bme.readPressure() / 100.0F; // hPa
  Humidity = bme.readHumidity();
}

void BtnDisplayPressed()
{
  btn_display_pressed = true;
}

void BtnSetupPressed()
{
  btn_setup_pressed = true;
}

void InitIOs(void)
{
  pinMode(LED_HIGH_CRIT, OUTPUT);
  pinMode(LED_HIGH_WARN, OUTPUT);
  pinMode(LED_NORMAL,    OUTPUT);
  pinMode(LED_LOW_WARN,  OUTPUT);
  pinMode(LED_LOW_CRIT,  OUTPUT);

  // Buttons are attached to external pullup resistors
  attachInterrupt(digitalPinToInterrupt(BUTTON_DISPLAY), BtnDisplayPressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_SETUP), BtnSetupPressed, FALLING);

  // give a short i am alive for all LEDs
  digitalWrite(LED_HIGH_CRIT, HIGH);
  digitalWrite(LED_HIGH_WARN, HIGH);
  digitalWrite(LED_NORMAL,    HIGH);
  digitalWrite(LED_LOW_WARN,  HIGH);
  digitalWrite(LED_LOW_CRIT,  HIGH);

  delay(500);
  
  AllStatusLEDsOff();
}

void AllStatusLEDsOff(void)
{
  digitalWrite(LED_HIGH_CRIT, LOW);
  digitalWrite(LED_HIGH_WARN, LOW);
  digitalWrite(LED_NORMAL,    LOW);
  digitalWrite(LED_LOW_WARN,  LOW);
  digitalWrite(LED_LOW_CRIT,  LOW);
}

void SetStatusLEDs(void)
{
  if (Humidity < 30)
  {
    digitalWrite(LED_LOW_CRIT,  HIGH);
    digitalWrite(LED_HIGH_CRIT, LOW);
    digitalWrite(LED_HIGH_WARN, LOW);
    digitalWrite(LED_NORMAL,    LOW);
    digitalWrite(LED_LOW_WARN,  LOW);
  } else if (Humidity < 40)
  {
    digitalWrite(LED_LOW_WARN,  HIGH);
    digitalWrite(LED_HIGH_CRIT, LOW);
    digitalWrite(LED_HIGH_WARN, LOW);
    digitalWrite(LED_NORMAL,    LOW);
    digitalWrite(LED_LOW_CRIT,  LOW);
  } else if (((DewPoint * 0.9) > Temperature) | (Humidity > 60))
  {
    digitalWrite(LED_HIGH_WARN, HIGH);
    digitalWrite(LED_HIGH_CRIT, LOW);
    digitalWrite(LED_NORMAL,    LOW);
    digitalWrite(LED_LOW_WARN,  LOW);
    digitalWrite(LED_LOW_CRIT,  LOW);
  } else if (((DewPoint * 0.95) > Temperature) | (Humidity > 70))
  {
    digitalWrite(LED_HIGH_CRIT, HIGH);
    digitalWrite(LED_HIGH_WARN, LOW);
    digitalWrite(LED_NORMAL,    LOW);
    digitalWrite(LED_LOW_WARN,  LOW);
    digitalWrite(LED_LOW_CRIT,  LOW);
  } else
  {
    digitalWrite(LED_NORMAL,    HIGH);
    digitalWrite(LED_HIGH_CRIT, LOW);
    digitalWrite(LED_HIGH_WARN, LOW);
    digitalWrite(LED_LOW_WARN,  LOW);
    digitalWrite(LED_LOW_CRIT,  LOW);
  }
}

void Display_Error(void)
{
  
  AllStatusLEDsOff();
  
  while(true)
  {
    digitalWrite(LED_HIGH_CRIT, !digitalRead(LED_HIGH_CRIT));
    digitalWrite(LED_LOW_CRIT,  !digitalRead(LED_LOW_CRIT)); 
    delay(500); 
  }
}

void BME_Sensor_Error(void)
{
  
  AllStatusLEDsOff();
  
  while(true)
  {
    digitalWrite(LED_HIGH_WARN, !digitalRead(LED_HIGH_CRIT));
    digitalWrite(LED_LOW_WARN,  !digitalRead(LED_LOW_CRIT)); 
    delay(500); 
  }
}

bool check_i2c_devices(byte i2c_address)
{
  byte wire_error_no;
  Wire.beginTransmission(i2c_address);
  wire_error_no = Wire.endTransmission();
  
  delay(10);
  
  if (wire_error_no != 0)
  {
    return false;
  }
    
  return true;
}

void setup() 
{
  Wire.begin();

  if (!check_i2c_devices(I2C_ADDR_OLED_DISPLAY))
  {
    Display_Error();
  }
  
  if (!check_i2c_devices(I2C_ADDR_BME280_SENSOR))
  {
    BME_Sensor_Error(); 
  }

  if (!InitBMESensor())
  {
    BME_Sensor_Error(); 
  }
  
  InitDisplay();
  InitIOs();
  
  delay(2000);
  getSensorValues();
  DewPoint = CalculateDewPointFast(Temperature, Humidity);
  PrintValuesDisplay();

  DisplayTimeout = 0;
  SensorTimeout = 0;
}

// This function is using a fast calculation of the dew point -> less accurate than the slow calculation.
float CalculateDewPointFast(float temperature, float humidity)
{
 float a = 17.271;
 float b = 237.7;
 float temp = (a * temperature) / (b + temperature) + log(humidity*0.01);
 float Td = (b * temp) / (a - temp);
 return Td;
}

// This function is using a slow calculation of the dew point -> much more accurate but also a very slow calculation.
float CalculateDewPointSlow(float temperature, float humidity) 
{
  float a = 7.5 ;    //für T >= 0
  float b = 237.3 ;  //für T >= 0
  float SDD = 6.1078 * pow(10,((a*temperature)/(b+temperature))); // Saturation vapor pressure [hPa]
  float DD = (humidity / 100) * SDD; // Vapor pressure [hPa]
  float v = log10((DD / 6.1078));    // Attention: use log10!
  return (b * v)/(a - v);            // Calculate dew point
}
 
void loop() 
{
  if (btn_display_pressed)
  {
    PrintValuesDisplay();
    SetStatusLEDs();
    btn_display_pressed = false;
  } else if (btn_setup_pressed)
  {
    // nothing defined yet
    btn_setup_pressed = false;
  }

  // 1000 x loop cycles (10ms delay in loop() => 10 sec display timeout)
  if (DisplayTimeout++ > 1000) 
  {
    display.clearDisplay();
    display.display();
    AllStatusLEDsOff();
    DisplayTimeout = 0;
  }

  // Trigger a sensor read every 1000 loop cycles
  if (SensorTimeout++ > 1000)
  {
    getSensorValues();
    DewPoint = CalculateDewPointFast(Temperature, Humidity);
    SensorTimeout = 0;
  }
  
  delay(10);
}
