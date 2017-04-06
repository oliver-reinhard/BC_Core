#include <ArduinoUnitX.h>
#include <BC_Log.h>

//#define DEBUG_UT_CONFIG

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  //Test::min_verbosity = TEST_VERBOSITY_ALL;
   Test::exclude("*");
   Test::include("a*");
  //Test::exclude("z_s_o_s");
}

void loop() {
  Test::run();
}


// ------ Unit Tests --------

test(a_data_type_sizes) {
  Serial.print(F("Log struct sizes (BC_Log.h): LogData: "));
  Serial.println(sizeof(LogData));
  Serial.print(F("LogMessageData: "));
  Serial.println(sizeof(LogMessageData));
  Serial.print(F("LogValuesData: "));
  Serial.println(sizeof(LogValuesData));
  Serial.print(F("LogStateData: "));
  Serial.println(sizeof(LogStateData));
  Serial.print(F("LogConfigParamData: "));
  Serial.println(sizeof(LogConfigParamData));
  
  assertLessOrEqual(sizeof(LogMessageData), sizeof(LogData));
  assertLessOrEqual(sizeof(LogValuesData), sizeof(LogData));
  assertLessOrEqual(sizeof(LogStateData), sizeof(LogData));
  assertLessOrEqual(sizeof(LogConfigParamData), sizeof(LogData));
}

test(b_logging) {
  RAMStore logStore = RAMStore(200);

  Log log = Log(&logStore);
  log.init();  
}