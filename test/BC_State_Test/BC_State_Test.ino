#ifndef UT_STATE_H_INCLUDED
  #define UT_STATE_H_INCLUDED
  
  #include <BC_Log.h>
  #include <BC_Control.h>
  
  // #define DEBUG_UT_STATE
  
  static const UserCommands ALL_CMD_INFO = CMD_INFO_HELP | CMD_INFO_CONFIG |  CMD_INFO_LOG | CMD_INFO_STAT;
  static const UserCommands ALL_CMD_CONFIG_MODIFY = CMD_CONFIG_SET_VALUE | CMD_CONFIG_SWAP_IDS | CMD_CONFIG_CLEAR_IDS | CMD_CONFIG_ACK_IDS | CMD_CONFIG_RESET_ALL;

  
  class MockLog : public Log {
    public:
      MockLog(AbstractStore *store) : Log(store) {}
      Timestamp logMessage(T_Message_ID, int16_t, int16_t) {    
        #ifdef DEBUG_UT_STATE
          Serial.println(F("DEBUG_UT_STATE: logMessage(...)"));
        #endif
        logMessageCount++;
        return logTime.timestamp();
      }
      
      Timestamp logValues(ACF_Temperature, ACF_Temperature, Flags)  {
        #ifdef DEBUG_UT_STATE
          Serial.println(F("DEBUG_UT_STATE: logValues(...)"));
        #endif
        logValuesCount++;
        return logTime.timestamp();
      }
      
      Timestamp logState(StateID, StateID, Event)  {
        #ifdef DEBUG_UT_STATE
          Serial.println(F("DEBUG_UT_STATE: logState(...)"));
        #endif
        logStateCount++;
        return logTime.timestamp();
      }
    
      // Mock counters:
      uint16_t logValuesCount = 0;
      uint16_t logStateCount = 0;
      uint16_t logMessageCount = 0;
      
      uint16_t totalInvocations() {
        return 
          logValuesCount +
          logStateCount +
          logMessageCount;
      }
      
      void resetCounters() {
        logValuesCount = 0;
        logStateCount = 0;
        logMessageCount = 0;
      }
  };


  class MockControlActions : public ControlActions {
    public:
      MockControlActions(ControlContext *context, UserFeedback *feedback) : ControlActions(context, feedback) { }
       
      // Mocked methods:
      void modifyConfig() {
        ControlActions::modifyConfig();
        modifyConfigCount++;
      }
      
      uint8_t setupSensors() { return 0; }
      void initSensorReadout() { }
      void completeSensorReadout() { }
  
      void heat(boolean on) {
        #ifdef DEBUG_UT_STATE
          Serial.println(F("DEBUG_UT_STATE: heat (on/off)"));
        #endif
        context->op->heating = on;
        if (on) {
          heatTrueCount++;
        } else {
          heatFalseCount++;
        }
      }
      
      // Mock counters:
      uint16_t heatTrueCount = 0;
      uint16_t heatFalseCount = 0;
      uint16_t modifyConfigCount = 0;
      
      uint16_t totalInvocations() {
        return 
          heatTrueCount +
          heatFalseCount +
          modifyConfigCount;
      }
      void resetCounters() {
        heatTrueCount = 0;
        heatFalseCount = 0;
        modifyConfigCount = 0;
      }
  };

  
#endif