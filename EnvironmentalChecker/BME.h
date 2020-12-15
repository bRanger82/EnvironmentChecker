#ifndef BME_HEADER

#define BME_HEADER
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define I2C_ADDR_BME280_SENSOR 0x76

float Temperature = 0;    // BME280 sensor value []
float Pressure    = 0;    // BME280 sensor value [hPa]
float Humidity    = 0;    // BME280 sensor value [rel %]
float DewPoint    = 0;    // Calculated based on the BME sensor values
  
Adafruit_BME280 bme;     // Using I2C interface

bool InitBMESensor(void)
{
  unsigned state = bme.begin(I2C_ADDR_BME280_SENSOR);
  if (!state)
  {
    return false;
  }
  return true;
}

#endif
