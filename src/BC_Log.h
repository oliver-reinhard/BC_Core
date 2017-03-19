#ifndef BC_LOG_H_INCLUDED
  #define BC_LOG_H_INCLUDED

  #if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_SAMD_MKR1000)
    #define LOG_DATA_PAYLOAD_SIZE 8
  #else
    #define LOG_DATA_PAYLOAD_SIZE 6
  #endif

  #include <ACF_Logging.h>
  #include <ACF_State.h>
  #include <BC_Config.h>
  
  typedef enum {
    LOG_DATA_TYPE_MESSAGE = 0,
    LOG_DATA_TYPE_VALUES = 1,
    LOG_DATA_TYPE_STATE = 2,
    LOG_DATA_TYPE_CONFIG = 3,
  } LogDataTypeEnum;


  /*
   * LogDataType: LOG_DATA_TYPE_MESSAGE, "subtype" of LogData
   */
  struct LogMessageData {
    T_Message_ID id;
    T_Message_Param params[2];
  };

  static_assert(sizeof(LogMessageData) <= sizeof(LogData), "LogMessageData > LogData");

  typedef uint8_t T_Flags; // value logging
  
  /*
   * LogDataType: LOG_DATA_TYPE_VALUES, "subtype" of LogData
   */
  struct LogValuesData {
    ACF_Temperature water;
    ACF_Temperature ambient;
    T_Flags flags;
  };

  static_assert(sizeof(LogValuesData) <= sizeof(LogData), "LogValuesData > LogData");

  /**
   * LogDataType: LOG_DATA_TYPE_STATE, "subtype" of LogData
   */
  struct LogStateData {
    T_State_ID previous;
    T_State_ID current;
    T_Event_ID event;
  };

  static_assert(sizeof(LogStateData) <= sizeof(LogData), "LogStateData > LogData");

  /**
   * LogDataType: LOG_DATA_TYPE_CONFIG, "subtype" of LogData
   */
  struct LogConfigParamData {
    T_ConfigParam_ID id;
    float newValue;
  };

  static_assert(sizeof(LogConfigParamData) <= sizeof(LogData), "LogConfigParamData > LogData");


  class Log : public AbstractLog {
    public:
      Log(AbstractStore *store) : AbstractLog(store) { };

      /*
       * Log a message.
       */
      virtual Timestamp logMessage(T_Message_ID msg, T_Message_Param param1, T_Message_Param param2);

      /*
       * Log a value change.
       */
      virtual Timestamp logValues(ACF_Temperature water, ACF_Temperature ambient, T_Flags flags);

      /*
       * Log a state change.
       */
      virtual Timestamp logState(StateID previous, StateID current, Event event);

      /*
       * Log a config-param change.
       */
      virtual Timestamp logConfigParam(T_ConfigParam_ID id, float newValue);     
  };

#endif
