#include <BC_Log.h>

//#define DEBUG_LOG

Timestamp Log::logMessage(T_Message_ID id, T_Message_Param param1, T_Message_Param param2) {
  #ifdef DEBUG_LOG
    Serial.println(F("DEBUG_LOG: logMessage(..)"));
  #endif
  LogMessageData data;
  memset(&data, 0x0, sizeof(data));
  data.id = id;
  data.params[0] = param1;
  data.params[1] = param2;
  const LogEntry e = addLogEntry(LOG_DATA_TYPE_MESSAGE, (LogData *) &data);
  return e.timestamp;
}

Timestamp Log::logValues(ACF_Temperature water, ACF_Temperature ambient, T_Flags flags) {
  #ifdef DEBUG_LOG
    Serial.println(F("DEBUG_LOG: logValues(..)"));
  #endif
  LogValuesData data;
  memset(&data, 0x0, sizeof(data));
  data.water = water;
  data.ambient = ambient;
  data.flags = flags;
  const LogEntry e = addLogEntry(LOG_DATA_TYPE_VALUES, (LogData *) &data);
  return e.timestamp;
}

Timestamp Log::logState(StateID previous, StateID current, Event event) {
  #ifdef DEBUG_LOG
    Serial.println(F("DEBUG_LOG: logState(..)"));
  #endif
  LogStateData data;
  memset(&data, 0x0, sizeof(data));
  data.previous = previous.id();
  data.current = current.id();
  data.event = event.id();
  const LogEntry e = addLogEntry(LOG_DATA_TYPE_STATE, (LogData *) &data);
  return e.timestamp;
}

Timestamp Log::logConfigParam(T_ConfigParam_ID id, float newValue) {
  #ifdef DEBUG_LOG
    Serial.println(F("DEBUG_LOG: logConfigParam(..)"));
  #endif
  LogConfigParamData data;
  memset(&data, 0x0, sizeof(data));
  data.id = id;
  data.newValue = newValue;
  const LogEntry e = addLogEntry(LOG_DATA_TYPE_CONFIG, (LogData *) &data);
  return e.timestamp;
}

