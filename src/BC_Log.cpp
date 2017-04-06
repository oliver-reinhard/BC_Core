#include <BC_Log.h>

//#define DEBUG_LOG

Timestamp Log::logMessage(T_Message_ID id, T_Message_Param param1, T_Message_Param param2) {
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: logMessage("));
    Serial.print(id);
    Serial.println(')');
  #endif
  LogMessageData data;
  memset(&data, 0x0, sizeof(data));
  data.id = id;
  data.params[0] = param1;
  data.params[1] = param2;
  const LogEntry e = addLogEntry(static_cast<T_LogDataType_ID>(LogDataType::MESSAGE), (LogData *) &data);
  return e.timestamp;
}

Timestamp Log::logValues(ACF_Temperature water, ACF_Temperature ambient, T_Flags flags) {
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: logValues("));
    Serial.print(water);
    Serial.print(',');
    Serial.print(ambient);
    Serial.println(')');
  #endif
  LogValuesData data;
  memset(&data, 0x0, sizeof(data));
  data.water = water;
  data.ambient = ambient;
  data.flags = flags;
  const LogEntry e = addLogEntry(static_cast<T_LogDataType_ID>(LogDataType::VALUES), (LogData *) &data);
  return e.timestamp;
}

Timestamp Log::logState(StateID previous, StateID current, Event event) {
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: logState("));
    Serial.print(previous.name());
    Serial.print(F("->["));
    Serial.print(event.name());
    Serial.print(F("]->"));
    Serial.print(current.name());
    Serial.println(')');
  #endif
  LogStateData data;
  memset(&data, 0x0, sizeof(data));
  data.previous = previous.id();
  data.current = current.id();
  data.event = event.id();
  const LogEntry e = addLogEntry(static_cast<T_LogDataType_ID>(LogDataType::STATE), (LogData *) &data);
  return e.timestamp;
}

Timestamp Log::logConfigParam(ConfigParam param, float newValue) {
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: logConfigParam("));
    Serial.print(static_cast<T_ConfigParam_ID>(param));
    Serial.print(':');
    Serial.print(newValue);
    Serial.println(')');
  #endif
  LogConfigParamData data;
  memset(&data, 0x0, sizeof(data));
  data.id = static_cast<T_ConfigParam_ID>(param);
  data.newValue = newValue;
  const LogEntry e = addLogEntry(static_cast<T_LogDataType_ID>(LogDataType::CONFIG), (LogData *) &data);
  return e.timestamp;
}

