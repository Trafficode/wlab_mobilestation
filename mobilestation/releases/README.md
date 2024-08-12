# v1_ncs2_5_3

## DONE
* SHT3x reading
* SIM800L network joining and MQTT publishing
* NVS for configuration storing
* Hardcoded APN to apn name TM
* Terminal commands for MQTT and WLAB configuration
* Timestamp properly read from GSM network and converted to UTC
* Publishing samples to WLAB server, properly presented on application side

## TODO
* Storing not properly published samples to flash memory and resending
* Handle LED indicator
* User button for future use
* Power saving mode
* Enter mcu to power saving mode
* Deeper sleep mode, cuting SHT3x sensor from power
* Reading and sending battery voltage value
* Terminal commands to store APN configuration
* BLE firmware update

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

## TODO
* Storing not properly published samples to flash memory and resending
* BLE firmware update
* Power saving modes
  * Extend publish period to 1h
  * Disable publishing till batter voltage to low
  * Cuting SHT3x sensor from power
* User button for future use
* Send battery voltage as additional publish data
