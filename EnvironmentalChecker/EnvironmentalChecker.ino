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

bool display_mode = true;

void PrintValuesDisplay(bool mode)
{
  display.ssd1306_command(SSD1306_DISPLAYON);
  PrintDataOnDisplay(Temperature, Pressure, Humidity, DewPoint, mode);
  DisplayTimeout = 0;
}

void getSensorValues(void)
{
  bool reading_ok = true;
  
#ifdef BME_FORCED_MODE
  reading_ok = bme.takeForcedMeasurement();
#endif

  if (reading_ok)
  {
    Temperature = bme.readTemperature();
    Pressure = bme.readPressure() / 100.0F; // hPa
    Humidity = bme.readHumidity();    
  } else
  {
    Temperature = 0.0f;
    Pressure = 0.0f;
    Humidity = 0.0f;
    BME_Sensor_Error();
  }
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

typedef enum state_e {eLOW_CRITICAL = 1, eLOW_WARNING = 2, eNORMAL = 3, eHIGH_WARNING = 4, eHIGH_CRITICAL = 5 } state_t;

void SetStatusLEDs(byte state)
{
  switch(state)
  {
    case eLOW_CRITICAL:
      digitalWrite(LED_HIGH_CRIT, LOW);
      digitalWrite(LED_HIGH_WARN, LOW);
      digitalWrite(LED_NORMAL,    LOW);
      digitalWrite(LED_LOW_WARN,  LOW);
      digitalWrite(LED_LOW_CRIT,  HIGH); 
      break;
    case eLOW_WARNING:
      digitalWrite(LED_HIGH_CRIT, LOW);
      digitalWrite(LED_HIGH_WARN, LOW);
      digitalWrite(LED_NORMAL,    LOW);
      digitalWrite(LED_LOW_WARN,  HIGH);
      digitalWrite(LED_LOW_CRIT,  LOW); 
      break;
    case eNORMAL:
      digitalWrite(LED_HIGH_CRIT, LOW);
      digitalWrite(LED_HIGH_WARN, LOW);
      digitalWrite(LED_NORMAL,    HIGH);
      digitalWrite(LED_LOW_WARN,  LOW);
      digitalWrite(LED_LOW_CRIT,  LOW); 
      break;  
    case eHIGH_WARNING:
      digitalWrite(LED_HIGH_CRIT, LOW);
      digitalWrite(LED_HIGH_WARN, HIGH);
      digitalWrite(LED_NORMAL,    LOW);
      digitalWrite(LED_LOW_WARN,  LOW);
      digitalWrite(LED_LOW_CRIT,  LOW);      
      break; 
    case eHIGH_CRITICAL:
      digitalWrite(LED_HIGH_CRIT, HIGH);
      digitalWrite(LED_HIGH_WARN, LOW);
      digitalWrite(LED_NORMAL,    LOW);
      digitalWrite(LED_LOW_WARN,  LOW);
      digitalWrite(LED_LOW_CRIT,  LOW);
      break;      
    default:
      digitalWrite(LED_LOW_CRIT,  LOW);
      digitalWrite(LED_HIGH_WARN, LOW);
      digitalWrite(LED_NORMAL,    LOW);
      digitalWrite(LED_LOW_WARN,  LOW);
      digitalWrite(LED_HIGH_CRIT, LOW);
      break; 
  }
}

void UpdateSetStatusLEDs(void)
{
  if ((Humidity > 70.0f) || ((DewPoint * 0.95) > Temperature))
  {
    SetStatusLEDs((byte)eHIGH_CRITICAL);
  } else if ((Humidity > 60.0f)  || ((DewPoint * 0.9) > Temperature))
  {
    SetStatusLEDs((byte)eHIGH_WARNING);
  } else if (Humidity < 30.0f)
  {
    SetStatusLEDs((byte)eLOW_CRITICAL);
  } else if (Humidity < 40.0f)
  {
    SetStatusLEDs((byte)eLOW_WARNING);
  } else
  {
    SetStatusLEDs((byte)eNORMAL);
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
  ACSR = (1 << ACD); //Disable the analog comparator
  DIDR0 = 0x3F; //Disable digital input buffers on all ADC0-ADC5 pins
  DIDR1 = (1 << AIN1D)|(1 << AIN0D); //Disable digital input buffer on AIN1/0
  power_adc_disable();
  power_spi_disable();
  
  set_sleep_mode (SLEEP_MODE_PWR_SAVE);
  sei();           // interrupts allowed now, next instruction WILL be executed
}

void EnterSleepMode(void)
{
  display.clearDisplay();
  display.display();
  delay(20);
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  set_sleep_mode(SLEEP_MODE_PWR_SAVE); // choose power down mode
  sleep_enable();
  sleep_mode();    // Now enter sleep mode.
}

void setup() 
{
  Serial.begin(19200);
  Serial.println(F("Startup environment checker ..."));
  
  Wire.begin();

  if (!check_i2c_devices(I2C_ADDR_OLED_DISPLAY))
  {
    Serial.println(F("I2C I2C_ADDR_OLED_DISPLAY failed!"));
    Display_Error();
  }
  
  if (!check_i2c_devices(I2C_ADDR_BME280_SENSOR))
  {
    Serial.println(F("I2C I2C_ADDR_BME280_SENSOR failed!"));
    BME_Sensor_Error(); 
  }

  if (!InitBMESensor())
  {
    Serial.println(F("InitBMESensor failed!"));
    BME_Sensor_Error(); 
  }

  Serial.print(F("Init display ... "));
  InitDisplay();
  Serial.println(F("done"));
  
  Serial.print(F("Init I/Os ... "));
  InitIOs();
  delay(50);
  AllStatusLEDs(false);
  Serial.println(F("done"));
  
  Serial.print(F("Reading values ... "));
  getSensorValues();
  DewPoint = CalculateDewPointFast(Temperature, Humidity);
  Serial.println(F("done"));

  Serial.print(F("Display values ... "));
  PrintValuesDisplay(true);
  UpdateSetStatusLEDs();
  Serial.println(F("done"));

  Serial.println(F("Startup done!"));
}

void ProcessSerialData(void)
{
  if (Serial.available() > 0)
  {
    int wert_ser = Serial.parseInt();
    switch(wert_ser)
    { 
      case 0:
        break;
      case 1:
        SetStatusLEDs((byte)wert_ser);
        break;
      case 2:
        SetStatusLEDs((byte)wert_ser);
        break;
      case 3:
        SetStatusLEDs((byte)wert_ser);
        break;
      case 4:
        SetStatusLEDs((byte)wert_ser);
        break;
      case 5:
        SetStatusLEDs((byte)wert_ser);
        break;
      case 6:
        AllStatusLEDs(true);
        break;
      case 7:
        AllStatusLEDs(false);
        break;
      case 8:
        getSensorValues();
        Serial.print(F("Temperature [°C]: "));
        Serial.println(Temperature);
        Serial.print(F("Pressure [hPa]: "));
        Serial.println(Pressure);
        Serial.print(F("Humidity [%]: "));
        Serial.println(Humidity);
        Serial.print(F("DewPoint [°C]: "));
        Serial.println(DewPoint);
        break;
      default:
        break;
    }
    if (wert_ser > 0 && wert_ser < 6)
    {
      Serial.print("Set ");
      Serial.println((byte)wert_ser);      
    }
    Serial.flush();
  }  
}

void loop() 
{
  ProcessSerialData();
  
  if (btn_display_pressed)
  {
    getSensorValues();
    DewPoint = CalculateDewPointFast(Temperature, Humidity);
    PrintValuesDisplay(display_mode);
    UpdateSetStatusLEDs();
    btn_display_pressed = false;
  } else if (btn_setup_pressed)
  {
    // nothing defined yet
    display_mode = !display_mode;
    btn_setup_pressed = false;
    EnterSleepMode();
  }

  // 1000 x loop cycles (10ms delay in loop() => 10 sec display timeout)
  if (DisplayTimeout++ > 1000) 
  {
    AllStatusLEDs(false);
    DisplayTimeout = 0;
    EnterSleepMode();
  }
  
  delay(10);
}
