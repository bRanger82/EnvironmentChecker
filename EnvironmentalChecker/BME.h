#ifndef BME_HEADER

#define BME_HEADER
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define I2C_ADDR_BME280_SENSOR 0x76

#define BME_FORCED_MODE true

float Temperature = 0;    // BME280 sensor value []
float Pressure    = 0;    // BME280 sensor value [hPa]
float Humidity    = 0;    // BME280 sensor value [rel %]
float DewPoint    = 0;    // Calculated based on the BME sensor values
  
Adafruit_BME280 bme;     // Using I2C interface

bool InitBMESensor(void)
{
  if (bme.begin(I2C_ADDR_BME280_SENSOR))
  {
#ifdef BME_FORCED_MODE
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X8, // temperature
                    Adafruit_BME280::SAMPLING_X8, // pressure
                    Adafruit_BME280::SAMPLING_X8, // humidity
                    Adafruit_BME280::FILTER_OFF,
                    Adafruit_BME280::STANDBY_MS_1000);
#endif
    return true;
  }
  return false;
}

bool GetSensorReading(float * temperature, float * humidity, float * pressure)
{
#ifdef BME_FORCED_MODE
  bool reading_ok = bme.takeForcedMeasurement();
#endif

  if (reading_ok)
  {
    *temperature = bme.readTemperature();
    *pressure = bme.readPressure() / 100.0f; // hPa
    *humidity = bme.readHumidity();    
  } else
  {
    *temperature = 0.0f;
    *pressure = 0.0f;
    *humidity = 0.0f;
    return false;
  }
  return true;
}

// This function is using a fast calculation of the dew point -> less accurate than the slow calculation.
float CalculateDewPointFast(float temperature, float humidity)
{
 float a = 17.271;
 float b = 237.7;
 float temp = (a * temperature) / (b + temperature) + log(humidity * 0.01);
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
  return (b * v) / (a - v);            // Calculate dew point
}

#endif
