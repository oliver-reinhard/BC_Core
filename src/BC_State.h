#ifndef BC_STATE_H_INCLUDED
  #define BC_STATE_H_INCLUDED
  
  #include <BC_Control.h>

  namespace States { 
    
    static const StateID INIT        = StateID(0, "Init");
    static const StateID SENSORS_NOK = StateID(1, "Sensors NOK");
    static const StateID READY       = StateID(2, "Ready"); 
    static const StateID IDLE        = StateID(3, "Idle"); 
    static const StateID RECORDING   = StateID(4, "Recording");
    static const StateID STANDBY     = StateID(5, "Standby"); 
    static const StateID HEATING     = StateID(6, "Heating"); 
    static const StateID OVERHEATED  = StateID(7, "Overheated");
    
    static const uint16_t NUM_STATES = 8;
    
    static constexpr const StateID *ALL_STATES[NUM_STATES] = { &INIT, &SENSORS_NOK, &READY, &IDLE, &RECORDING, &STANDBY, &HEATING, &OVERHEATED };
    
    /*
     * Find a state ID object given its scalar value.
     */
    const StateID findState(const T_State_ID id);
  }

  namespace Events {
    
    const Event NONE          = EVENT_NONE;                  // 1
    const Event READY         = Event(0x1, "Ready");         // 2
    const Event SENSORS_NOK   = Event(0x2, "Sensors NOK");   // 3
    const Event INFO          = Event(0x4, "Info");          // 4
    const Event CONFIG_MODIFY = Event(0x8, "Config Modify"); // 5     (8)
    const Event CONFIG_RESET  = Event(0x10, "Config Reset"); // 6    (16)
    const Event REC_ON        = Event(0x20, "Rec On");       // 7    (32)
    const Event REC_OFF       = Event(0x40, "Rec Off");      // 8    (64)
    const Event HEAT_ON       = Event(0x80, "Heat On");      // 9   (128)
    const Event HEAT_OFF      = Event(0x100, "Heat Off");    // 10  (256)
    const Event HEAT_RESET    = Event(0x200, "Heat Reset");  // 11  (512)
    const Event TEMP_OVER     = Event(0x400, "Temp Over");   // 12 (1024)
    const Event TEMP_OK       = Event(0x800, "Temp OK");     // 13 (2048)
  
    const uint16_t NUM_EVENTS = 13;
  
    static constexpr const Event *ALL_EVENTS[NUM_EVENTS] = {&NONE, &READY, &SENSORS_NOK, &INFO, &CONFIG_MODIFY, &CONFIG_RESET, &REC_ON, &REC_OFF, &HEAT_ON, &HEAT_OFF, &HEAT_RESET, &TEMP_OVER, &TEMP_OK };
    
    /*
     * Find an event object given its scalar value.
     */
    const Event findEvent(const T_Event_ID id);
  
    /*
     * Maps state-automaton events to user commands.
     * Note: not all events have corresponding user commands (=> CMD_NONE), but all user commands map to an event
     */
    const UserCommands EVENT_CMD_MAP [NUM_EVENTS] = {
      CMD_NONE,                                                                               // 1 - NONE
      CMD_NONE,                                                                               // 2 - READY
      CMD_NONE,                                                                               // 3 - SENSORS_NOK
      CMD_INFO_HELP | CMD_INFO_STAT | CMD_INFO_CONFIG | CMD_INFO_LOG,                         // 4 - INFO
      CMD_CONFIG_SET_VALUE | CMD_CONFIG_SWAP_IDS | CMD_CONFIG_CLEAR_IDS | CMD_CONFIG_ACK_IDS, // 5 - CONFIG_MODIFY
      CMD_CONFIG_RESET_ALL,                                                                   // 6 - CONFIG_RESET
      CMD_REC_ON,                                                                             // 7 - REC_ON
      CMD_REC_OFF,                                                                            // 8 - REC_OFF
      CMD_HEAT_ON,                                                                            // 9 - HEAT_ON
      CMD_HEAT_OFF,                                                                           // 10 - HEAT_OFF
      CMD_HEAT_RESET,                                                                         // 11 - HEAT_RESET
      CMD_NONE,                                                                               // 12 - TEMP_OVER
      CMD_NONE                                                                                // 13 - TEMP_OK
    };
    
    /*
     * All events (except NONE) ordered by descending priority, i.e. most urgent first.
     */
    static constexpr const Event *EVENT_PRIORITIES [NUM_EVENTS] = {
      &TEMP_OVER,     // 1
      &TEMP_OK,       // 2
      &READY,         // 3
      &SENSORS_NOK,   // 4
      &HEAT_OFF,      // 5
      &HEAT_ON,       // 6
      &REC_OFF,       // 7
      &REC_ON,        // 8
      &HEAT_RESET,    // 9
      &CONFIG_RESET,  // 10
      &CONFIG_MODIFY, // 11
      &INFO,          // 12
      &NONE           // 13
    };
  }

  class ExecutionContext : public ControlContext {
    public:
      ControlActions *control;
  };
  
  class ContextAware {
    public:
      void setContext(ExecutionContext *context) { this->context = context; }
    protected:
      ExecutionContext *context;
  };

  
  class Init : public AbstractSimpleState, public ContextAware {
    public:
      StateID id() { return States::INIT; }
      // No user commands for this state.
      EventSet eval(const TimeMillis timeInState, const Event userRequest = EVENT_NONE);
    
      // On startup, wait at most this time before for sensors to initialise properery, then automatically transition to SensorsNOK.
      static const TimeMillis SENSOR_INIT_TIMEOUT_MS = 10000L;
    
    protected:
      StateID transAction(Event event);
  };
  
  
  class SensorsNOK : public AbstractSimpleState, public ContextAware {
    public:
      StateID id() { return States::SENSORS_NOK; }
      EventSet acceptedUserEvents();
      
    protected:
      StateID transAction(Event event);
  };

  
  class Ready : public AbstractCompositeState, public ContextAware {
    public:      
      StateID id() { return States::READY; }
      EventSet acceptedUserEvents();
      
    protected:
      StateID transAction(Event event);
  };

  
  class Idle : public AbstractSimpleState, public ContextAware {
    public:
      StateID id() { return States::IDLE; }
      EventSet acceptedUserEvents();
      
    protected:
      StateID transAction(Event event);
  };

  
  class Recording : public AbstractCompositeState, public ContextAware {
    public:
      StateID id() { return States::RECORDING; }
      EventSet acceptedUserEvents();
      
    protected:
      StateID transAction(Event event);
      
      /*
       * Turn value logging ON (entry) / OFF (exit)
       */
      void entryAction();
      void exitAction();
  };

  
  class Standby : public AbstractSimpleState, public ContextAware {
    public:
      StateID id() { return States::STANDBY; }
      EventSet acceptedUserEvents();
      
    protected:
      StateID transAction(Event event);
  };

  
  class Heating : public AbstractSimpleState, public ContextAware {
    public:
      StateID id() { return States::HEATING; }
      EventSet acceptedUserEvents();
      EventSet eval(const TimeMillis timeInState, const Event userRequest = EVENT_NONE);
      
    protected:
      StateID transAction(Event event);
      
      /*
       * Turn heater ON (entry) / OFF (exit)
       */
      void entryAction();
      void exitAction();
  };

  
  class Overheated : public AbstractSimpleState, public ContextAware {
    public:
      StateID id() { return States::OVERHEATED; }
      EventSet acceptedUserEvents();
      EventSet eval(const TimeMillis timeInState, const Event userRequest = EVENT_NONE);
      
    protected:
      StateID transAction(Event event);
  };

  
  /*
   * STATE AUTOMATON
   */
  class BoilerStateAutomaton : public AbstractStateAutomaton {
    public:
    
      void init(ExecutionContext *context);
    
      /*
       * Returns the set of user commands supported by the current state of the automaton.
       */
      UserCommands acceptedUserCommands();

      /*
       * Map a user command to its corresponding event using EVENT_CMD_MAP.
       * @return NONE if no mapping is found
       */
      Event commandToEvent(UserCommandEnum command);
    
    private:
      ExecutionContext *context;
          
      Init INIT;
      Idle IDLE;
      SensorsNOK SENSORS_NOK;
      Standby STANDBY;
      Heating HEATING;
      Overheated OVERHEATED;
      Recording RECORDING;
      AbstractState *RECORDING_SUBSTATES[3] = {&STANDBY, &HEATING, &OVERHEATED};
      Ready READY;
      AbstractState *READY_SUBSTATES[2] = {&IDLE, &RECORDING};

      AbstractState *ALL_STATES[States::NUM_STATES] = {&INIT, &IDLE, &SENSORS_NOK, &STANDBY, &HEATING, &OVERHEATED, &RECORDING, &READY};
      
      void stateChanged(StateID fromState, Event event, StateID toState);

  #ifdef UNIT_TEST
    public:
  #endif
      
      /*
       * Map a set of events ot their corresponding user commands using EVENT_CMD_MAP.
       * @return CMD_NONE if no mapping is found
       */
      UserCommands eventsToCommands(EventSet events);
  };
  
#endif
