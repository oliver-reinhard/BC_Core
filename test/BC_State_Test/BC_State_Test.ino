#include <ArduinoUnitX.h>
#include <BC_State.h>
#include "ut_state.h"

#define LOG_SIZE 200

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  //Test::min_verbosity = TEST_VERBOSITY_ALL;
  //Test::exclude("*");
  //Test::include("a*");
  //Test::exclude("z_s_o_s");
}

void loop() {
  Test::run();
}


// ------ Unit Tests --------
test(a_state_automaton_sensors_ok) {
  RAMStore configStore = RAMStore(sizeof(ConfigParams));
  RAMStore logStore = RAMStore(LOG_SIZE);

  ConfigParams config = ConfigParams(&configStore);
  config.load();
  
  #ifdef DEBUG_UT_STATE
    config.print();
  #endif
  assertEqual(config.heaterCutOutWaterTemp, DEFAULT_HEATER_CUT_OUT_WATER_TEMP); 

  MockLog logger = MockLog(&logStore);
  logger.clear();
    
  
  OperationalParams op;
  op.request.clear();
  
  ExecutionContext context;
  UserFeedback feedback;
  MockControlActions control = MockControlActions(&context, &feedback);

  context.log = &logger;
  context.config = &config;
  context.op = &op;
  context.control = &control;
  
  BoilerStateAutomaton automaton;
  automaton.init(&context);

  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::INIT.id()));
  assertEqual(int16_t(automaton.acceptedUserCommands()), int16_t(CMD_NONE));
  assertEqual(control.totalInvocations(), 0);

  // water sensor = DS18B20_SENSOR_INITIALISING => evaluate => no event
  assertEqual(int16_t(context.op->water.sensorStatus), int16_t(DS18B20_SENSOR_INITIALISING));
  EventSet candidates = automaton.evaluate(/*no user event*/);
  assertEqual(candidates.events(), Events::NONE.id());
  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::INIT.id()));
  assertEqual(control.totalInvocations(), 0);

  // transition (INVALID event) => stay in state INIT
  automaton.transition(Events::CONFIG_MODIFY); 
  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::INIT.id()));
  assertEqual(control.totalInvocations(), 0);
  // invalid event logging
  assertEqual(logger.logMessageCount, 1);
  assertEqual(logger.totalInvocations(), 1);
  logger.resetCounters();

  // transition again (INVALID event) => must NOT LOG again
  automaton.transition(Events::CONFIG_MODIFY); 
  assertEqual(logger.totalInvocations(), 0);

  //
  // set water sensor OK => evaluate => event READY
  //
  op.water.sensorStatus = DS18B20_SENSOR_OK;
  op.water.currentTemp = 2300;
  candidates = automaton.evaluate(/* no user event */);
  assertEqual(candidates.events(), Events::READY.id());
  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::INIT.id()));
  assertEqual(control.totalInvocations(), 0);
  assertEqual(logger.totalInvocations(), 0);
  

  // transition state INIT => event READY => state IDLE
  automaton.transition(Events::READY);
  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::IDLE.id()));
  assertEqual(control.totalInvocations(), 0);
  assertEqual(int16_t(op.loggingValues), int16_t(false));
  assertEqual(int16_t(op.heating), int16_t(false));
  // state logging
  assertEqual(logger.logStateCount, 1);
  assertEqual(logger.totalInvocations(), 1);
  logger.resetCounters();

  //
  // user command CONFIG_MODIFY in state IDLE
  //
  assertEqual(int16_t(automaton.acceptedUserCommands()), int16_t(ALL_CMD_INFO | ALL_CMD_CONFIG_MODIFY | CMD_CONFIG_RESET_ALL | CMD_REC_ON));
  candidates = automaton.evaluate(Events::CONFIG_MODIFY);
  assertEqual(candidates.events(), Events::CONFIG_MODIFY.id());
  
  // transition state IDLE => event CONFIG_MODIFY => stay in state IDLE
  automaton.transition(Events::CONFIG_MODIFY); 
  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::IDLE.id()));
  assertEqual(control.modifyConfigCount, 1);
  assertEqual(control.totalInvocations(), 1);
  control.resetCounters();

  //
  // user command REC ON in state IDLE
  //
  candidates = automaton.evaluate(Events::REC_ON);
  assertEqual(candidates.events(), Events::REC_ON.id());
  
  // transition state IDLE => event REC_ON => state STANDBY
  assertEqual(int16_t(op.loggingValues), int16_t(false));
  automaton.transition(Events::REC_ON); 
  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::STANDBY.id()));
  assertEqual(int16_t(op.loggingValues), int16_t(true));
  assertEqual(int16_t(op.heating), int16_t(false));
  control.resetCounters();

  //
  // user command REC OFF in state STANDBY
  //
  assertEqual(int16_t(automaton.acceptedUserCommands()), int16_t(ALL_CMD_INFO | CMD_REC_OFF | CMD_HEAT_ON));
  candidates = automaton.evaluate(Events::REC_OFF);
  assertEqual(candidates.events(), Events::REC_OFF.id());
  
  // transition state STANDBY => event REC_OFF => state IDLE
  automaton.transition(Events::REC_OFF); 
  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::IDLE.id()));
  assertEqual(int16_t(op.loggingValues), int16_t(false));
  assertEqual(int16_t(op.heating), int16_t(false));
  control.resetCounters();

  // transition state IDLE => event REC_ON => state STANDBY
  automaton.transition(Events::REC_ON);
  control.resetCounters();
  
  //
  // user command HEAT ON in state STANDBY
  //
  candidates = automaton.evaluate(Events::HEAT_ON);
  assertEqual(candidates.events(), Events::HEAT_ON.id());
  
  // transition state STANDBY => event HEAT_ON => state HEATING
  automaton.transition(Events::HEAT_ON); 
  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::HEATING.id()));
  assertEqual(control.heatTrueCount, 1);
  assertEqual(control.totalInvocations(), 1);
  assertEqual(int16_t(op.loggingValues), int16_t(true));
  assertEqual(int16_t(op.heating), int16_t(true));
  control.resetCounters();

  // evaluate => no event
  assertEqual(config.heaterCutOutWaterTemp, DEFAULT_HEATER_CUT_OUT_WATER_TEMP); 
  candidates = automaton.evaluate(/* no user event */);
  assertEqual(candidates.events(), Events::NONE.id());

  //
  // set water-sensor temp to overheated
  //
  op.water.currentTemp = DEFAULT_HEATER_CUT_OUT_WATER_TEMP + 1;
  candidates = automaton.evaluate(/* no user event */);
  assertEqual(candidates.events(), Events::TEMP_OVER.id());

  // transition state HEATING => event TEMP_OVER => state OVERHEATED
  automaton.transition(Events::TEMP_OVER); 
  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::OVERHEATED.id()));
  assertEqual(control.heatFalseCount, 1);
  assertEqual(control.totalInvocations(), 1);
  assertEqual(int16_t(op.loggingValues), int16_t(true));
  assertEqual(int16_t(op.heating), int16_t(false));
  control.resetCounters();
  
  // evaluate => no event
  candidates = automaton.evaluate(/* no user event */);
  assertEqual(candidates.events(), Events::NONE.id());

  //
  // set water-sensor temp to ok
  //
  op.water.currentTemp = DEFAULT_HEATER_BACK_OK_WATER_TEMP -1;
  assertEqual(config.heaterBackOkWaterTemp, DEFAULT_HEATER_BACK_OK_WATER_TEMP); 
  candidates = automaton.evaluate(/* no user event */);
  assertEqual(candidates.events(), Events::TEMP_OK.id());
  op.water.currentTemp = 4500;

  // transition state OVERHEATED => event TEMP_OK => state HEATING
  automaton.transition(Events::TEMP_OK); 
  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::HEATING.id()));
  assertEqual(control.heatTrueCount, 1);
  assertEqual(control.totalInvocations(), 1);
  assertEqual(int16_t(op.loggingValues), int16_t(true));
  assertEqual(int16_t(op.heating), int16_t(true));
  control.resetCounters();
  
  //
  // user command HEAT OFF in state HEATING
  //
  candidates = automaton.evaluate(Events::HEAT_OFF);
  assertEqual(candidates.events(), Events::HEAT_OFF.id());
  
  // transition state HEATING => event HEAT_OFF => state STANDBY
  automaton.transition(Events::HEAT_OFF); 
  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::STANDBY.id()));
  assertEqual(control.heatFalseCount, 1);
  assertEqual(control.totalInvocations(), 1);
  assertEqual(int16_t(op.loggingValues), int16_t(true));
  assertEqual(int16_t(op.heating), int16_t(false));
  control.resetCounters();
}


test(b_state_automaton_sensors_nok) {
  RAMStore configStore = RAMStore(sizeof(ConfigParams));
  RAMStore logStore = RAMStore(LOG_SIZE);
  
  ConfigParams config = ConfigParams(&configStore);
  config.load();
  MockLog logger = MockLog(&logStore);
  logger.clear();
  logger.resetCounters();
  
  OperationalParams op;
  op.request.clear();
  
  ExecutionContext context;
  UserFeedback feedback;
  MockControlActions control = MockControlActions(&context, &feedback);

  context.log = &logger;
  context.config = &config;
  context.op = &op;
  context.control = &control;

  BoilerStateAutomaton automaton;
  automaton.init(&context);

  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::INIT.id()));
  assertEqual(int16_t(automaton.acceptedUserCommands()), int16_t(CMD_NONE));
  assertEqual(control.totalInvocations(), 0);

  //
  // set water sensor NOK => evaluate => event SENSORS_NOK
  //
  op.water.sensorStatus = DS18B20_SENSOR_NOK;
  EventSet candidates = automaton.evaluate(/*no user event*/);
  assertEqual(candidates.events(), Events::SENSORS_NOK.id());
  
  // transition state INIT => event SENSORS_NOK => state SENSORS_NOK
  automaton.transition(Events::SENSORS_NOK);
  assertEqual(int16_t(automaton.state()->id().id()), int16_t(States::SENSORS_NOK.id()));
  assertEqual(int16_t(automaton.acceptedUserCommands()), int16_t(ALL_CMD_INFO | ALL_CMD_CONFIG_MODIFY | CMD_CONFIG_RESET_ALL));
  assertEqual(control.totalInvocations(), 0);
  // state logging
  assertEqual(logger.logStateCount, 1);
  assertEqual(logger.totalInvocations(), 1);
  logger.resetCounters();
}