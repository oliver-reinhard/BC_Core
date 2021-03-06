#ifndef BC_CONTROL_H_INCLUDED
  #define BC_CONTROL_H_INCLUDED

  #include <ACF_Types.h>
  #include <BC_Config.h>
  #include <BC_Log.h>

  #define HEATER_PIN    9
  #define ONE_WIRE_PIN  10 // Temperature sensors

  enum class ControlMsg : T_Message_ID {
    WATER_TEMP_SENSOR_SILENT = 20,      // Water-temperature sensor not responding. Check wiring and sensor, then restart.
    WATER_TEMP_SENSOR_ID_UNDEF = 21,    // Sensor ID of water-temperature sensor could not be obtained. Check wiring and sensor, then restart.
    WATER_TEMP_SENSOR_ID_AUTO =  22,    // Sensor ID of water-temperature sensor was assigned automatically. Confirm that it refers to the correct sensor, then restart.
    AMBIENT_TEMP_SENSOR_SILENT = 23,    // Ambient-temperature sensor not responding. Check wiring and sensor, then restart.
    AMBIENT_TEMP_SENSOR_ID_UNDEF = 24,  // Sensor ID of ambient-temperature sensor could not be obtained. Check wiring and sensor, then restart.
    AMBIENT_TEMP_SENSOR_ID_AUTO =  25,  // Sensor ID of ambient-temperature sensor was assigned automatically. Confirm that it refers to the correct sensor, then restart.
    TEMP_SENSOR_IDS_CLEARED =  26       // Sensor IDs of water- and ambient-temperature sensor cleared. Restart to see the effect.
  };


  // Water min and max values used to check that sensor-temperature readout is plausible:
  #define WATER_MIN_TEMP -2000 // [°C * 100]
  #define WATER_MAX_TEMP 10000 // [°C * 100]
  
  // Ambient min and max values used to check that ambient-temperature readout is plausible:
  #define AMBIENT_MIN_TEMP -3000 // [°C * 100]
  #define AMBIENT_MAX_TEMP 5000 // [°C * 100]
  
  #define CMD_ARG_BUF_SIZE 30   // Size of the read buffer for incoming data
  #define CMD_ARG_VALUE_SIZE 4  // Size of command argument values

  typedef uint16_t T_UserCommand_ID;

  typedef enum {
    CMD_NONE             = 0,       // 1
    CMD_INFO_HELP        = 0x1,     // 2
    CMD_INFO_STAT        = 0x2,     // 3
    CMD_INFO_CONFIG      = 0x4,     // 4
    CMD_INFO_LOG         = 0x8,     // 5
    CMD_CONFIG_SET_VALUE = 0x10,    // 6   (16)
    CMD_CONFIG_SWAP_IDS  = 0x20,    // 7   (32)
    CMD_CONFIG_CLEAR_IDS = 0x40,    // 8   (64)
    CMD_CONFIG_ACK_IDS   = 0x80,    // 9   (128)
    CMD_CONFIG_RESET_ALL = 0x100,   // 10  (256)
    CMD_REC_ON           = 0x200,   // 11  (512)
    CMD_REC_OFF          = 0x400,   // 12  (1024)
    CMD_HEAT_ON          = 0x800,   // 13  (2048)
    CMD_HEAT_OFF         = 0x1000,  // 14  (4096)
    CMD_HEAT_RESET       = 0x2000,  // 15  (8192)
  } UserCommandEnum;
  
  const uint8_t NUM_USER_COMMANDS = 15;

  /*
   * ID for enum type.
   */
  const T_UserCommand_ID MAX_USER_COMMAND_ID = 2 << (NUM_USER_COMMANDS - 1);

  // Bitwise OR combination ("|") of UserCommandEnum(s):
  typedef uint16_t UserCommands;

  /*
   * Representation of a user control request via a command:
   */
  struct UserRequest {
    UserCommandEnum command;
    ConfigParam param;
    int32_t intValue;
    float floatValue;
    Event event = EVENT_NONE;  // event that corresponds to command

    void clear() {
      command = CMD_NONE;
      param = ConfigParam::NONE;
      intValue = INT32_MIN;
      floatValue = -999999;
      event = EVENT_NONE;
    }

    void setCommand(T_UserCommand_ID id) {
      ASSERT(id <= MAX_USER_COMMAND_ID, "command id");
      command = UserCommandEnum(id);
    }

    void setParamValue(ConfigParam p, int32_t value) {
      command = CMD_CONFIG_SET_VALUE;
      param = p;
      intValue = value;
    }

    void setParamValue(ConfigParam p, float value) {
      command = CMD_CONFIG_SET_VALUE;
      param = p;
      floatValue = value;
    }
  };

  class UserFeedback {
    public:
      virtual void commandExecuted(boolean /* success */) { }
  };

  
  struct OperationalParams {
    
    /* Timepoint [ms] of most recent transition to current state.  */
    TimeMillis currentStateStartMillis = 0L;

    /* Representation of the physical water-temperature sensor. */
    DS18B20_Sensor water = DS18B20_Sensor("Water");
    
    /* Representation of the physical ambient-temperature sensor.*/
    DS18B20_Sensor ambient = DS18B20_Sensor("Ambient");

    /* A control request made by the user. */
    UserRequest request;

    /* Is the boiler heating element on or off? */
    boolean heating = false;
    
    /*  Time [ms] of most recent transition to state HEATING.*/
    TimeMillis heatingStartMillis = 0L;
    
    /* Accumulated time in state HEATING, not including the period since last start (if heatingStartMillis != 0L). */
    TimeMillis heatingAccumulatedMillis = 0L;

    /* Time [s] to reach target temperature when heating first started. This value doesn't change during heating. */
    TimeSeconds originalTimeToGo = UNDEFINED_TIME_SECONDS;
    
    /* Are we logging value changes such as measured temperatures? */
    boolean loggingValues = false;

    /* Swap sensor IDs and sensor states between water and ambient sensor. */
    void swapDS18B20_SensorIDs();
  };

  /*
   * Calculates the accumulated heating time.
   */
  TimeMillis heatingTotalMillis(OperationalParams *op);
  

  class ControlContext {
    public:
      Log *log;
      ConfigParams *config;
      OperationalParams *op;
      DS18B20_Controller *controller;

      /*
       * Calculates and returns the time to reach target temperature based on
       *   - current water temperature
       *   - target temperature
       *   - water mass
       *  @return UNDEFINED_TIME_SECONDS if a value cannot be calculated (e.g. because the water-temperature sensor is NOK)
       */
       TimeSeconds originalTimeToGo();
  };

  /*
   * This class is stateless; all effects of actions are either immediate (to a port of the Arduino chip, including serial com port), or 
   * they change operational parameter values.
   * Note: it is implemented as a class instead of normal functions for unit-testing purposes (mocking).
   */
  class ControlActions {
    public:
      ControlActions(ControlContext *context, UserFeedback *feedback) {
        this->context = context;
        this->userFeedback = feedback;
      }

      /*
       * Changes config parameters based on user requests (context->op->request).
       * Provides feedback about the success of the modification via the UserFeedback object.
       */
      virtual void modifyConfig();
      
      /*
       * Performs a search on the OneWire Bus and matches / assigns returned sensor addresses with configured sensors.
       */
      uint8_t setupSensors();

      /*
       * Sends a command to all physical temperature sensors to trigger the conversion of physical values to a temperature.
       * 
       * Note: The conversion takes time (up to 750 ms depending on the precision of the returned value) and completeSensorReadout cannot be invoked before that.
       *       See the data sheet of the DS18B20 temperature sensors for exact times.
       */
      void initSensorReadout();

      /*
       * Reads the temperatures from the physical sensors and assignes the value and state to the configured temperature sonsors.
       * 
       * Note: See the note for initSensorReadout()
       */
      void completeSensorReadout();

      /*
       * Physically turns the water heater on or off.
       */
      virtual void heat(boolean on);
      
    protected:
      ControlContext *context;
      UserFeedback *userFeedback;

      /*
       * Logs a problem that arose during sensor setup.
       */
      void logSensorSetupIssue(DS18B20_Sensor *sensor, ControlMsg msg);
      
      /*
       * Sets a config param value and logs the change.
       * @return true if value was set and logged, false else
       */
      boolean setConfigParamValue(ConfigParam p, int32_t intValue, float floatValue);

      /*
       * Swap the sensor IDs and states of water- and ambient-temperature sensor in the operational parameters.
       */
      void swapDS18B20_SensorIDs();

      /*
       * Clears the sensor IDs of water- and ambient-temperature sensors in the configuration parameters and saves the latter to persistent storage. 
       */
      void clearDS18B20_SensorIDs();
      
      /*
       * Copies the sensor IDs of water- and ambient-temperature sensors of the the configuration parameters to the configuration parameters and saves the latter to persistent storage. 
       * Sets the sensor status of one or both sensors to DS18B20_SENSOR_OK iff sensor status is DS18B20_SENSOR_ID_AUTO_ASSIGNED.
       * 
       * @return true if at lestt one ID was copied and status was changed.
       */
      boolean confirmDS18B20_SensorIDs();
  };
  
#endif
