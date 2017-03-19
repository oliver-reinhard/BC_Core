#include <ArduinoUnitX.h>
#include <BC_Config.h>

//#define DEBUG_UT_CONFIG

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  //Test::min_verbosity = TEST_VERBOSITY_ALL;
  // Test::exclude("*");
  // Test::include("e*");
  //Test::exclude("z_s_o_s");
}

void loop() {
  Test::run();
}


// ------ Unit Tests --------

test(a_config_load_save) {
  RAMStore configStore = RAMStore(sizeof(ConfigParams));

  ConfigParams config = ConfigParams(&configStore);
  config.load();
  
  #ifdef DEBUG_UT_CONFIG
    config.print();
  #endif
  assertEqual(config.targetTemp, DEFAULT_TARGET_TEMP);
 
  ACF_Temperature newTemp = 4400; // [°C * 100]
  config.targetTemp = newTemp;
  config.save();

  ConfigParams config1 = ConfigParams(&configStore);
  config1.load();
  
  #ifdef DEBUG_UT_CONFIG
    config.print();
  #endif
  assertEqual(config.targetTemp, newTemp);
}