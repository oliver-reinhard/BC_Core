#include <BC_Control.h>

// #define DEBUG_CONTROL

#define WATER_HEAT_CAPACITY 4182.0 // J / (kg*K)

TimeMillis heatingTotalMillis(OperationalParams *op) {
  TimeMillis duration = op->heatingAccumulatedMillis;
  if (op->heatingStartMillis != 0L) {
    duration += millis() - op->heatingStartMillis;
  }
  return duration;
}

/*
 * OPERATIONAL PARAMS
 */

void OperationalParams::swapDS18B20_SensorIDs() {
  DS18B20_SensorID tempId;
  memcpy(tempId, water.id, DS18B20_SENSOR_ID_BYTES);
  water.setId(ambient.id);
  ambient.setId(tempId);
  
  DS18B20_StatusEnum tempStatus = water.sensorStatus;
  water.sensorStatus = ambient.sensorStatus;
  ambient.sensorStatus = tempStatus;      
}


/*
 * CONTROL CONTEXT
 */
 
TimeSeconds ControlContext::originalTimeToGo() {
  if (op->water.sensorStatus == DS18B20_SENSOR_OK) {
    float tempDiff =  ((float) (config->targetTemp - op->water.currentTemp)) / 100.0;
    if (tempDiff > 0.0) {
      return config->tankCapacity * WATER_HEAT_CAPACITY * tempDiff / config->heaterPower;
    }
  }
  return UNDEFINED_TIME_SECONDS;
}

/*
 * CONTROL ACTIONS
 */

void ControlActions::modifyConfig() {
  UserRequest *request = &(context->op->request);
  UserCommandEnum command = request->command;
  if (command == CMD_CONFIG_SET_VALUE) {
    boolean success = setConfigParamValue(request->param, request->intValue, request->floatValue);
    if (success) {
      context->config->save();
    }
    userFeedback->commandExecuted(success);
    
  } else if (command == CMD_CONFIG_SWAP_IDS) {
    swapDS18B20_SensorIDs();
    userFeedback->commandExecuted(true);
    
  } else if (command == CMD_CONFIG_CLEAR_IDS) {
    clearDS18B20_SensorIDs();
    userFeedback->commandExecuted(true);
    
  } else if (command == CMD_CONFIG_ACK_IDS) {
    boolean success = confirmDS18B20_SensorIDs();
    userFeedback->commandExecuted(success);
    
  } else {
    userFeedback->commandExecuted(false);
  }
}
 
boolean ControlActions::setConfigParamValue(ConfigParam p, int32_t intValue, float floatValue) {
  ConfigParams *config = context->config;
  Log *log = context->log;
  switch(p) {
    case ConfigParam::TARGET_TEMP: 
      {
        ACF_Temperature targetTemp = (ACF_Temperature) intValue;
        if (config->targetTemp != targetTemp) {
          config->targetTemp = targetTemp;
          log->logConfigParam(p, (float) config->targetTemp);
        }
      }
      return true;
      
    case ConfigParam::HEATER_CUT_OUT_WATER_TEMP: 
      {
        ACF_Temperature heaterCutOutWaterTemp = (ACF_Temperature) intValue;
        if (config->heaterCutOutWaterTemp != heaterCutOutWaterTemp) {
          config->heaterCutOutWaterTemp = heaterCutOutWaterTemp;
          log->logConfigParam(p, (float) config->heaterCutOutWaterTemp);
        }
      }
      return true;
      
    case ConfigParam::HEATER_BACK_OK_WATER_TEMP: 
      {
        ACF_Temperature heaterBackOkWaterTemp = (ACF_Temperature) intValue;
        if (config->heaterBackOkWaterTemp != heaterBackOkWaterTemp) {
          config->heaterBackOkWaterTemp = heaterBackOkWaterTemp;
          log->logConfigParam(p, (float) config->heaterBackOkWaterTemp);
        }
      }
      return true;
      
    case ConfigParam::LOG_TEMP_DELTA: 
      {
        ACF_Temperature logTempDelta = (ACF_Temperature) intValue;
        if (config->logTempDelta != logTempDelta) {
          config->logTempDelta = logTempDelta;
          log->logConfigParam(p, (float) config->logTempDelta);
        }
      }
      return true;
      
    case ConfigParam::LOG_TIME_DELTA:
      {
        uint16_t logTimeDelta = (uint16_t) intValue;
        if (config->logTimeDelta != logTimeDelta) {
          config->logTimeDelta = logTimeDelta;
          log->logConfigParam(p, (float) config->logTimeDelta);
        }
      }
      return true;
      
    case ConfigParam::TANK_CAPACITY:
      {
        float tankCapacity = floatValue;
        if (config->tankCapacity != tankCapacity) {
          config->tankCapacity = tankCapacity;
          log->logConfigParam(p, config->tankCapacity);
        }
      }
      return true;
      
    case ConfigParam::HEATER_POWER:
      {
        float heaterPower = floatValue;
        if (config->heaterPower != heaterPower) {
          config->heaterPower = floatValue;
          log->logConfigParam(p, config->heaterPower);
        }
      }
      return true;
      
    default:
      return false;
  }
}

void ControlActions::swapDS18B20_SensorIDs() {
  context->op->swapDS18B20_SensorIDs();
}

void ControlActions::clearDS18B20_SensorIDs() {
  context->config->setDS18B20_SensorIDs((uint8_t*) DS18B20_UNDEFINED_SENSOR_ID, (uint8_t*) DS18B20_UNDEFINED_SENSOR_ID);
  context->config->save();
  context->log->logConfigParam(ConfigParam::WATER_TEMP_SENSOR_ID, 0.0);
  context->log->logConfigParam(ConfigParam::AMBIENT_TEMP_SENSOR_ID, 0.0);
  context->log->logMessage(static_cast<T_Message_ID>(ControlMsg::TEMP_SENSOR_IDS_CLEARED), 0, 0);
}

boolean ControlActions::confirmDS18B20_SensorIDs() {
  if (context->op->water.sensorStatus == DS18B20_SENSOR_ID_AUTO_ASSIGNED || context->op->ambient.sensorStatus == DS18B20_SENSOR_ID_AUTO_ASSIGNED) {
    context->config->setDS18B20_SensorIDs(context->op->water.id, context->op->ambient.id);
    context->config->save();
    context->log->logConfigParam(ConfigParam::WATER_TEMP_SENSOR_ID, 0.0);
    context->log->logConfigParam(ConfigParam::AMBIENT_TEMP_SENSOR_ID, 0.0);
    
    context->op->water.confirmId();
    context->op->ambient.confirmId();
    return true;
  }
  return false;
}


uint8_t ControlActions::setupSensors() {
  DS18B20_Sensor *waterSensor = &context->op->water;
  waterSensor->setId(context->config->waterTempSensorId);
  waterSensor->rangeMin = WATER_MIN_TEMP;
  waterSensor->rangeMax = WATER_MAX_TEMP;
  
  DS18B20_Sensor *ambientSensor = &context->op->ambient;
  ambientSensor->setId(context->config->ambientTempSensorId);
  ambientSensor->rangeMin = AMBIENT_MIN_TEMP;
  ambientSensor->rangeMax = AMBIENT_MAX_TEMP;
  
  uint8_t matched = context->controller->setupSensors();
  
  if (waterSensor->sensorStatus == DS18B20_SENSOR_INITIALISING) {
    logSensorSetupIssue(waterSensor, ControlMsg::WATER_TEMP_SENSOR_SILENT); 
  } else if (waterSensor->sensorStatus == DS18B20_SENSOR_ID_UNDEFINED) {
    logSensorSetupIssue(waterSensor, ControlMsg::WATER_TEMP_SENSOR_ID_UNDEF); 
  } else if (waterSensor->sensorStatus == DS18B20_SENSOR_ID_AUTO_ASSIGNED) {
    logSensorSetupIssue(waterSensor, ControlMsg::WATER_TEMP_SENSOR_ID_AUTO); 
  }
  
  if (ambientSensor->sensorStatus == DS18B20_SENSOR_INITIALISING) {
    logSensorSetupIssue(ambientSensor, ControlMsg::AMBIENT_TEMP_SENSOR_SILENT); 
  } else if (ambientSensor->sensorStatus == DS18B20_SENSOR_ID_UNDEFINED) {
    logSensorSetupIssue(ambientSensor, ControlMsg::AMBIENT_TEMP_SENSOR_ID_UNDEF); 
  } else if (ambientSensor->sensorStatus == DS18B20_SENSOR_ID_AUTO_ASSIGNED) {
   logSensorSetupIssue(ambientSensor, ControlMsg::AMBIENT_TEMP_SENSOR_ID_AUTO); 
  } 

   #ifdef DEBUG_CONTROL
     Serial.print(F("DEBUG_CONTROL: Sensors matched or found = "));
     Serial.println(matched);
   #endif
  return matched;
}
void ControlActions::initSensorReadout() {
  context->controller->initSensorReadout();
}


void ControlActions::completeSensorReadout() {
  context->controller->completeSensorReadout();
}

void ControlActions::heat(boolean on) {
  context->op->heating = on;
  if (on) {
    digitalWrite(HEATER_PIN, HIGH);
  } else {
    digitalWrite(HEATER_PIN, LOW);
  }
}

void ControlActions::logSensorSetupIssue(DS18B20_Sensor *sensor, ControlMsg msg) {
  context->log->logMessage(static_cast<T_Message_ID>(msg), 0, 0);
  #ifdef DEBUG_CONTROL
    Serial.print(F("DEBUG_CONTROL: "));
    Serial.print(sensor->label);
    Serial.print(F(" sensor status = "));
    Serial.println(sensor->sensorStatus);
  #else
    if (sensor != NULL) { } // prevent 'unused parameter' warning
  #endif
}
