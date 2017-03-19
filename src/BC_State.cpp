#include <BC_State.h>

  #define DEBUG_STATE


const StateID States::findState(const T_State_ID id) {
  for(uint16_t i = 0; i<NUM_STATES; i++) {
    if (ALL_STATES[i]->id() == id) {
      return *ALL_STATES[i];
    }
  }
  return STATE_UNDEFINED;
}

const Event Events::findEvent(const T_Event_ID id) {
  for(uint16_t i = 0; i<NUM_EVENTS; i++) {
    if (ALL_EVENTS[i]->id() == id) {
      return *ALL_EVENTS[i];
    }
  }
  return Events::NONE;
}

  
/*
 * INIT
 */

// No user events available, but automatic events.

EventSet Init::eval(const Event /* userRequest */) {
  EventSet result = AbstractState::eval();
  if (context->op->water.sensorStatus == DS18B20_SENSOR_INITIALISING) {
    result |= Events::NONE;  // = wait
  } else if (context->op->water.sensorStatus == DS18B20_SENSOR_OK) {
    result |= Events::READY;
  } else  {
    result |= Events::SENSORS_NOK;
  } 
  return result;
}

StateID Init::transAction(Event event) {
  if (event == Events::READY) {
    return States::READY;
  } else if (event == Events::SENSORS_NOK) {
    return States::SENSORS_NOK;
  }
  return AbstractState::transAction(event);
}

/*
 * SENSORS NOK
 */

EventSet SensorsNOK::acceptedUserEvents() {
  return AbstractState::acceptedUserEvents() | Events::INFO | Events::CONFIG_MODIFY | Events::CONFIG_RESET;
}

// No events or conditions other than user-requested transitions => user super.eval();

StateID SensorsNOK::transAction(Event event) {
  if (event == Events::INFO) {
    return STATE_SAME;
    
  } else if (event == Events::CONFIG_MODIFY) {
    context->control->modifyConfig();
    return STATE_SAME;
    
  } else if (event == Events::CONFIG_RESET) {
    context->config->reset();
    context->control->setupSensors();
    return STATE_SAME;   
  }
  return AbstractState::transAction(event);
}

/*
 * READY
 */

EventSet Ready::acceptedUserEvents() {
  return AbstractState::acceptedUserEvents() | Events::INFO;
}

// No events or conditions other than user-requested transitions => user super.eval();

StateID Ready::transAction(Event event) {
  if (event == Events::INFO) {
    return STATE_SAME;
    
  } else if (event == Events::SENSORS_NOK) {
    return States::SENSORS_NOK;
  }
  return AbstractState::transAction(event);
}

/*
 * IDLE
 */

EventSet Idle::acceptedUserEvents() {
  return AbstractState::acceptedUserEvents() | Events::CONFIG_MODIFY | Events::CONFIG_RESET | Events::REC_ON;
}

// No events or conditions other than user-requested transitions => user super.eval();

StateID Idle::transAction(Event event) {
  if (event == Events::CONFIG_MODIFY) {
    context->control->modifyConfig();
    return STATE_SAME;
    
  } else if (event == Events::CONFIG_RESET) {
    context->config->reset();
    context->control->setupSensors();
    return States::SENSORS_NOK;
    
  } else if (event == Events::REC_ON) {
    return States::RECORDING;
  }
  return AbstractState::transAction(event);
}

/*
 * RECORDING
 */

EventSet Recording::acceptedUserEvents() {
  return AbstractState::acceptedUserEvents() | Events::REC_OFF;
}

// No events or conditions other than user-requested transitions => user super.eval();

StateID Recording::transAction(Event event) {
  if (event == Events::REC_OFF) {
    return States::IDLE;
  }
  return AbstractState::transAction(event);
}

void Recording::entryAction() {
  context->op->loggingValues = true;
  context->op->originalTimeToGo = context->originalTimeToGo();
}

void Recording::exitAction(){
  context->op->loggingValues = false;
  context->op->originalTimeToGo = UNDEFINED_TIME_SECONDS;
}

/*
 * STANDBY
 */

EventSet Standby::acceptedUserEvents() {
  return AbstractState::acceptedUserEvents() | Events::HEAT_ON;
}

// No events or conditions other than user-requested transitions => user super.eval();

StateID Standby::transAction(Event event) {
  if (event == Events::HEAT_ON) {
    return States::HEATING;
  }
  return AbstractState::transAction(event);
}

/*
 * HEATING
 */

EventSet Heating::acceptedUserEvents() {
  return AbstractState::acceptedUserEvents() | Events::HEAT_OFF;
}

EventSet Heating::eval(const Event userRequest = EVENT_NONE) {
  EventSet result = AbstractState::eval(userRequest); // handles user-requested events
  if (context->op->water.currentTemp >= context->config->heaterCutOutWaterTemp) {
    result |= Events::TEMP_OVER;
  }
  return result;
}

StateID Heating::transAction(Event event) {
  if (event == Events::HEAT_OFF) {
    return States::STANDBY;
  } else if (event == Events::TEMP_OVER) {
    return States::OVERHEATED;
  }
  return AbstractState::transAction(event);
}

void Heating::entryAction() {
  context->control->heat(true);
  context->op->heatingStartMillis = millis();
}

void Heating::exitAction(){
  context->control->heat(false);
  context->op->heatingAccumulatedMillis += millis() - context->op->heatingStartMillis;
  context->op->heatingStartMillis = 0L;
}

/*
 * OVERHEATED
 */

EventSet Overheated::acceptedUserEvents() {
  return AbstractState::acceptedUserEvents() | Events::HEAT_RESET;
}

EventSet Overheated::eval(const Event /* userRequest */) {
  EventSet result = AbstractState::eval();  // handles user-requested events
  if (context->op->water.currentTemp <= context->config->heaterBackOkWaterTemp) {
    result |= Events::TEMP_OK;
  }
  return result;
}

StateID Overheated::transAction(Event event) {
  if (event == Events::HEAT_RESET) {
    return States::STANDBY;
    
  } else if (event == Events::TEMP_OK) {
    return States::HEATING;
  }
  return AbstractState::transAction(event);
}

/*
 * STATE AUTOMATON
 */
void BoilerStateAutomaton::init(ExecutionContext *context) {
  this->context = context;
  
  INIT.setContext(context);
  IDLE.setContext(context);
  SENSORS_NOK.setContext(context);
  STANDBY.setContext(context);
  HEATING.setContext(context);
  OVERHEATED.setContext(context);
  RECORDING.setContext(context);
  RECORDING.setSubstates(RECORDING_SUBSTATES, 3);
  READY.setContext(context);
  READY.setSubstates(READY_SUBSTATES, 2);
  
  setStates(ALL_STATES, States::NUM_STATES);
  currentState = &INIT;
}
 
UserCommands BoilerStateAutomaton::acceptedUserCommands() {
  EventSet acceptedEvents = currentState->acceptedUserEvents();
  return eventsToCommands(acceptedEvents);
}

Event BoilerStateAutomaton::commandToEvent(UserCommandEnum command) {
  Event event = Events::NONE;
  // Skip event Events::NONE (index i=0):
  for(uint8_t i=1; i<Events::NUM_EVENTS; i++) {
    if (command & Events::EVENT_CMD_MAP[i]) {
      event = 1 << (i-1);
    }
  }
  return (Event) event;
}

UserCommands BoilerStateAutomaton::eventsToCommands(EventSet events) {
  UserCommands commands = CMD_NONE;
  // Skip event Events::NONE (index i=0):
  T_Event_ID eventId = 1;
  for(uint8_t i=1; i<Events::NUM_EVENTS; i++) {
    if (eventId & events.events()) {
      commands |= Events::EVENT_CMD_MAP[i];
    }
    eventId = eventId << 1;
  }
  return commands;
}

void BoilerStateAutomaton::stateChanged(const StateID fromState, const Event event, const StateID toState) {
    context->op->currentStateStartMillis = millis();
    context->log->logState(fromState, toState, event);
}

