#ifndef BC_LOG_H_INCLUDED
  #define BC_LOG_H_INCLUDED

  #include <ACF_Logging.h>
  #include <ACF_State.h>
  #include <BC_Config.h>
  
  /*
   * The messages issued by the implementation of this module. Stored as T_Message_ID.
   * Note: the actual message texts and the conversion of message IDs to text have to be implemented by 
   *       the consumer of this libraray 
   */
  typedef enum {
    MSG_LOG_DATA_SIZE = 10   // Size of LogData subtype does not correspond to sizeof(LogData); subtype = [LogDataTypeEnum]
  } LogMessageEnum;
  
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
    int16_t params[2];
    T_Message_ID id;
  };
  
  typedef uint8_t Flags; // value logging
  
  /*
   * LogDataType: LOG_DATA_TYPE_VALUES, "subtype" of LogData
   */
  struct LogValuesData {
    ACF_Temperature water;
    ACF_Temperature ambient;
    Flags flags;
  };
  
  /**
   * LogDataType: LOG_DATA_TYPE_STATE, "subtype" of LogData
   */
  struct LogStateData {
    T_State_ID previous;
    T_State_ID current;
    T_Event_ID event;
  };
  
  /**
   * LogDataType: LOG_DATA_TYPE_CONFIG, "subtype" of LogData
   */
  struct LogConfigParamData {
    T_ConfigParam_ID id;
    float newValue;
  };

  
  class Log : public AbstractLog {
    public:
      Log(AbstractStore *store) : AbstractLog(store) { };

      /*
       * Log a message.
       */
      virtual Timestamp logMessage(T_Message_ID msg, int16_t param1, int16_t param2);

      /*
       * Log a value change.
       */
      virtual Timestamp logValues(ACF_Temperature water, ACF_Temperature ambient, Flags flags);

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
