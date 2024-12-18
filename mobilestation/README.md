# Configuration
* deviceid 1100203040AB       // Set DeviceId
* pubp 10                     // Publish period in seconds
* mqttconf 10.190.1.20 1883   // Mqtt broker access
* apn internet user pass      // Configure APN
* apn internet


# Configuration over BLE(NUS)
* pubp 10                     // Publish period in seconds
* mqttconf 10.190.1.20 1883   // Mqtt broker access
* apn internet user pass      // Configure APN
* apn internet
* deviceid 0A112233ABFF       // Set device id
* pconfig
* help

# LED scenes
* BLE enabled at startup: Fast blinking for 20 seconds - don't stop once connection established
* Startup network init success: Quick led blink
* Startup network init failed: Two led blinks with 1 second interval
* Sensor read success: Quick led blink(heart beat every 20 seconds)
* Sensor read failed: Two led blinks with 1 second interval
* Publish sample to broker success: No led action
* Publish sample to broker failed: Three led blinks with 1 second interval

# Release process
* Add update and merged files to releases directory
* Write down description into releases/README.md file
* Trigger up version in version.h
* Create tag as version name

# TODO
* Storing not properly published samples to persistant storage
* Enable DC/DC instead of LDO
* Why modem is blinking
* Power saving modes
  * Extend publish period to 1h
  * Disable publishing till batter voltage to low
  * Cuting SHT3x sensor from power
* User button for future use
* Send battery voltage as additional publish data
* MqttDash android app
  
# v5_ncs2_7_0

## DONE
* Critical change, CIPSEND with predefined data length. Previous solution caused failed in sending samples that includes sensitive bytes.
* Set device id using BLE NUS
* Verify cpu usage, growing when publishing, <1% otherwise
* Switch from toolchain 16.1 to 2.7.0
* Storing not properly published samples to temporary/RAM storage
* Sim800l commands: builtin retries, some command returns error from time to time
  * AT+CSTT="internet","","" 
  * AT+CIPSTATUS
* Publishing with QoS = 1

# v4_ncs2_7_0

## DONE
* BLE adv enabled for 20 seconds since start. If not connected, disable and
  operate normaly
* BLE firmware update, custom .pem key added(CMakeLists.txt)
* Configuration over BLE(NUS), implemented commands:
  * pubp 10                     // Publish period in seconds
  * mqttconf 10.190.1.20 1883   // Mqtt broker access
  * apn internet user pass      // Configure APN
  * apn internet
  * pconfig
  * help
* Sim800l network up improved
* Extended parition for user non-volatile data


# v3_ncs2_7_0

## DONE
* SIM800l entering to sleep mode fixed. Power consumption of SIM800l in sleep
mode is ~1mA. MCU and the rest of hardware consume ~1.4mA. 


# v2_ncs2_7_0

## DONE
* Handle LED indicator
  * Startup network init success: Quick led blink
  * Startup network init failed: Two led blinks with 1 second interval
  * Sensor read success: Quick led blink(heart beat every 20 seconds)
  * Sensor read failed: Two led blinks with 1 second interval
  * Publish sample to broker success: No led action
  * Publish sample to broker failed: Three led blinks with 1 second interval
* Reading and sending battery voltage value(temporary is send as Humidity)
* Terminal commands to store APN configuration
* Comunication with sim800l improved, redundant '\0' removed from AT commands


# v1_ncs2_5_3

## DONE
* SHT3x reading
* SIM800L network joining and MQTT publishing
* NVS for configuration storing
* Hardcoded APN to apn name TM
* Terminal commands for MQTT and WLAB configuration
* Timestamp properly read from GSM network and converted to UTC
* Publishing samples to WLAB server, properly presented on application side












