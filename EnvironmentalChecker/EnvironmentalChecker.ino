#include <Wire.h>
#include <avr/sleep.h>
#include <avr/power.h>
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

void PrintValuesDisplay(void)
{
  PrintDataOnDisplay(Temperature, Pressure, Humidity, DewPoint);
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
  sleep_disable();
  power_all_enable();
  btn_display_pressed = true;
}

void BtnSetupPressed()
{
  sleep_disable();
  power_all_enable();
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
  
  AllStatusLEDs(false);
}

void AllStatusLEDs(bool On)
{
  if (On)
  {
    digitalWrite(LED_HIGH_CRIT, HIGH);
    digitalWrite(LED_HIGH_WARN, HIGH);
    digitalWrite(LED_NORMAL,    HIGH);
    digitalWrite(LED_LOW_WARN,  HIGH);
    digitalWrite(LED_LOW_CRIT,  HIGH);    
  } else
  {
    digitalWrite(LED_HIGH_CRIT, LOW);
    digitalWrite(LED_HIGH_WARN, LOW);
    digitalWrite(LED_NORMAL,    LOW);
    digitalWrite(LED_LOW_WARN,  LOW);
    digitalWrite(LED_LOW_CRIT,  LOW);    
  }
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
  
  AllStatusLEDs(false);
  
  while(true)
  {
    digitalWrite(LED_HIGH_CRIT, !digitalRead(LED_HIGH_CRIT));
    digitalWrite(LED_LOW_CRIT,  !digitalRead(LED_LOW_CRIT)); 
    delay(500); 
  }
}

void BME_Sensor_Error(void)
{
  
  AllStatusLEDs(false);
  
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

void SetupSleepMode(void)
{
  // disable ADC
  ADCSRA &= ~(1<<ADEN); //Disable ADC
  ACSR = (1<<ACD); //Disable the analog comparator
  DIDR0 = 0x3F; //Disable digital input buffers on all ADC0-ADC5 pins
  DIDR1 = (1<<AIN1D)|(1<<AIN0D); //Disable digital input buffer on AIN1/0
  power_adc_disable();
  power_spi_disable();
  
  set_sleep_mode (SLEEP_MODE_PWR_SAVE);
  sei();           // interrupts allowed now, next instruction WILL be executed
}

void EnterSleepMode(void)
{
  set_sleep_mode(SLEEP_MODE_PWR_SAVE); // choose power down mode
  sleep_enable();
  // Now enter sleep mode.
  sleep_mode();  
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
  
  delay(500);
  
  getSensorValues();
  DewPoint = CalculateDewPointFast(Temperature, Humidity);
  PrintValuesDisplay();
}

void loop() 
{
  if (btn_display_pressed)
  {
    getSensorValues();
    DewPoint = CalculateDewPointFast(Temperature, Humidity);
    PrintValuesDisplay();
    SetStatusLEDs();
    btn_display_pressed = false;
  } else if (btn_setup_pressed)
  {
    // nothing defined yet
    btn_setup_pressed = false;
    // do work
    AllStatusLEDs(true);
    delay(250);
    AllStatusLEDs(false);
    EnterSleepMode();
  }

  // 1000 x loop cycles (10ms delay in loop() => 10 sec display timeout)
  if (DisplayTimeout++ > 1000) 
  {
    display.clearDisplay();
    display.display();
    AllStatusLEDs(false);
    DisplayTimeout = 0;
    EnterSleepMode();
  }
  
  delay(10);
}
