#include <BC_Config.h>


void ConfigParams::setDS18B20_SensorIDs(DS18B20_SensorID water, DS18B20_SensorID ambient) {
  memcpy(this->waterTempSensorId, water, DS18B20_SENSOR_ID_BYTES);
  memcpy(this->ambientTempSensorId, ambient, DS18B20_SENSOR_ID_BYTES);
}


void ConfigParams::reset() {
  clear();
  boolean updated;
  initParams(updated);
}


void ConfigParams::initParams(boolean &updated) {
  updated = false;
  
  if (targetTemp == 0) {
    targetTemp = DEFAULT_TARGET_TEMP;
    updated = true;
  }

  // Temp Sensor IDs do not have a default value => need to be set as part of the installation.

  if (heaterCutOutWaterTemp == 0) {
    heaterCutOutWaterTemp = DEFAULT_HEATER_CUT_OUT_WATER_TEMP;
    updated = true;
  }

  if (heaterBackOkWaterTemp == 0) {
    heaterBackOkWaterTemp = DEFAULT_HEATER_BACK_OK_WATER_TEMP;
    updated = true;
  }

  if (logTempDelta == 0) {
    logTempDelta = DEFAULT_LOG_TEMP_DELTA;
    updated = true;
  }

  if (logTimeDelta == 0L) {
    logTimeDelta = DEFAULT_LOG_TIME_DELTA;
    updated = true;
  }
  
  if (tankCapacity == 0.0) {
    tankCapacity = DEFAULT_TANK_CAPACITY;
    updated = true;
  }

  if (heaterPower == 0.0) {
    heaterPower = DEFAULT_HEATER_POWER;
    updated = true;
  }

  // *** Initialise new config parameters here:
}


ConfigParamType ConfigParams::paramType(ConfigParam param) {
  switch(param) {
    case ConfigParam::WATER_TEMP_SENSOR_ID: 
    case ConfigParam::AMBIENT_TEMP_SENSOR_ID:
      return ConfigParamType::TEMP_SENSOR_ID;
    case ConfigParam::TARGET_TEMP:
    case ConfigParam::HEATER_CUT_OUT_WATER_TEMP:
    case ConfigParam::HEATER_BACK_OK_WATER_TEMP:
    case ConfigParam::LOG_TEMP_DELTA:
      return ConfigParamType::TEMPERATURE;
    case ConfigParam::LOG_TIME_DELTA:
    case ConfigParam::TANK_CAPACITY:
    case ConfigParam::HEATER_POWER:
      return ConfigParamType::FLOAT;
    default:
      return ConfigParamType::UNDEFINED;
  }
}

void ConfigParams::print() {
  char buf[max(MAX_TEMPERATURE_STR_LEN, MAX_DS18B20_SENSOR_ID_STR_LEN)];
  AbstractConfigParams::print();
  Serial.println(formatTemperature(targetTemp, buf));
  Serial.println(formatDS18B20_SensorID(waterTempSensorId, buf));
  Serial.println(formatDS18B20_SensorID(ambientTempSensorId, buf));
  Serial.println(formatTemperature(heaterCutOutWaterTemp, buf));
  Serial.println(formatTemperature(heaterBackOkWaterTemp, buf));
  Serial.println(formatTemperature(logTempDelta, buf));
  Serial.println(logTimeDelta);
  Serial.println(tankCapacity);
  Serial.println(heaterPower);
}

