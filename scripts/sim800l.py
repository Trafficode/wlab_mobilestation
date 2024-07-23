import serial
import time

# modprobe ftdi_sio
# modprobe usbserial

# Terminal macro file v2
# AT$0D 41540Dh         Test
# AT&F                  Factory settings
# ATE0                  Echo OFF
# AT+CPIN?              Check SIM card status
# AT+CSQ                Signal quality check
# AT+CGATT=1            Attach to GPRS service
# AT+CLTS=1             Enable the network time synchronization
# AT+CCLK?              Check the current time: +CCLK: "21/07/17,12:34:56+00"
# AT+CIPSHUT            Reset IP session
# AT+CIPSTATUS          Check IP status
# AT+CIPMUX=0           Set single connection mode
# AT+CSTT="TM","",""    Set APN's
# AT+CIICR              Bring up wireless connection
# AT+CIFSR              Get IP address (expecting a non-empty response)
# AT+CREG?              Check gprs conection expected response: "+CREG: 0,1" "+CREG: 0,5"

# TCP connection to broker start
# AT+CIPSTART="TCP","194.42.111.14","1883"
# AT+CIPSTART="TCP","test.mosquitto.org","1883"

# PING
# AT+CIPPING="8.8.8.8"

# AT+CIPSEND Connect
# 10 0C 00 04 4D 51 54 54 04 02 00 3C 00 00 1A
# 10 13 00 04 4d 51 54 54 04 02 00 3c 00 07 62 69 62 61 31 32 33 1A    
# 10 13 00 04 4D 51 54 54 04 02 00 3C 00 07 62 69 62 61 31 32 33 1A     # biba123
# 10 10 00 04 4D 51 54 54 04 02 00 3C 00 04 44 49 47 49 1A              # DIGI

# AT+CIPSEND Connect
# 10 10 00 04 4D 51 54 54 04 02 00 3C 00 04 44 49 47 49 1A  # DIGI
# AT+CIPSEND Publish
# 30 10 00 0B 2F 77 6C 61 62 64 62 2F 62 69 6E 31 32 33 1A  # message: 123, topic /wlabdb/bin
# AT+CIPSEND
# 30 15 00 0C 2F 77 6C 61 62 64 62 2F 74 65 73 74 54 65 73 74 2E 2E 2E 1A 

# AT+CIPSEND
# 10 0C 00 04 4D 51 54 54 04 02 00 3C 00 00 30 15 00 0C 2F 77 6C 61 62 64 62 2F 74 65 73 74 54 65 73 74 2E 2E 2F 30 15 00 0C 2F 77 6C 61 62 64 62 2F 74 65 73 74 54 65 73 74 2E 2E 2A 1A

# AT+CIPCLOSE

# AT+GMR   What is firmware version
# Revision:1418B04SIM800L24

def send_at_command(ser, command, expected_response, timeout=2):
    ser.write((command + '\r').encode())
    time.sleep(timeout)
    response = ser.read_all()
    # Filter out non-ASCII bytes
    # response = bytes([b for b in response if 32 <= b <= 126 or b in (10, 13)])  # Include printable ASCII and newline, carriage return
    response = response.decode('ascii').strip()  # Decode to ASCII
    print(f"Sent: {command}, Received: {response}")
    if expected_response:
        return expected_response in response, response
    else:
        return True, response

def enable_data_roaming(ser, apn):
    commands = [
        {"command": 'AT+CREG?', "expected_response": "+CREG"},
        {"command": 'AT+SAPBR=3,1,"Contype","GPRS"', "expected_response": "OK"},
        {"command": f'AT+SAPBR=3,1,"APN","{apn}"', "expected_response": "OK"},
        {"command": 'AT+COPS=0', "expected_response": "OK"},
        {"command": 'AT+CGATT=1', "expected_response": "OK"},
        {"command": 'AT+SAPBR=1,1', "expected_response": "OK"},
        {"command": 'AT+SAPBR=2,1', "expected_response": "+SAPBR"},
    ]

    for cmd in commands:
        success, response = send_at_command(ser, cmd["command"], cmd["expected_response"])
        if not success:
            print(f"Command '{cmd['command']}' failed with response: {response}")
            return False
        else:
            print(f"Command '{cmd['command']}' succeeded with response: {response}")
    return True

def configure_gprs(ser, apn):
    print('*** configure_gprs')
    commands = [
        {"command": "AT", "expected_response": "OK"},
        {"command": "ATE0", "expected_response": "OK"},  # Echo off
        {"command": 'AT+SAPBR=0,1', "expected_response": None},
        {"command": 'AT+SAPBR=3,1,"Contype","GPRS"', "expected_response": "OK"},
        {"command": f'AT+SAPBR=3,1,"APN","{apn}"', "expected_response": "OK"},
        {"command": "AT+SAPBR=1,1", "expected_response": "OK"},
        {"command": "AT+SAPBR=2,1", "expected_response": "OK"},
    ]

    for cmd in commands:
        success, response = send_at_command(ser, cmd["command"], cmd["expected_response"])
        if not success:
            print(f"Command '{cmd['command']}' failed with response: {response}")
            return False
        else:
            print(f"Command '{cmd['command']}' succeeded with response: {response}")
    return True

def configure_modem(ser, apn):
    commands = [
        {"command": "AT", "expected_response": "OK"},
        {"command": "AT&F", "expected_response": "OK"},
        {"command": "ATE0", "expected_response": "OK"},  # Echo off
        {"command": "AT+CPIN?", "expected_response": "+CPIN: READY"},  # Check SIM card status
        {"command": "AT+CSQ", "expected_response": "OK"},  # Signal quality check
        {"command": "AT+CGATT=1", "expected_response": "OK"},  # Attach to GPRS service
        {"command": "AT+CIPSHUT", "expected_response": "SHUT OK"},  # Reset IP session
        {"command": "AT+CIPSTATUS", "expected_response": "OK"},  # Check IP status
        {"command": "AT+CIPMUX=0", "expected_response": "OK"},  # Set single connection mode
        {"command": f"AT+CSTT=\"{apn}\",\"\",\"\"", "expected_response": "OK"},  # Set APN
        {"command": "AT+CIICR", "expected_response": "OK"},  # Bring up wireless connection
        {"command": "AT+CIFSR", "expected_response": ""},  # Get IP address (expecting a non-empty response)
    ]

    for cmd in commands:
        success, response = send_at_command(ser, cmd["command"], cmd["expected_response"])
        if not success:
            print(f"Command '{cmd['command']}' failed with response: {response}")
            return False
        else:
            print(f"Command '{cmd['command']}' succeeded with response: {response}")
    return True

def ping(ser):
    # Perform a ping test to Google's DNS server
    success, response = send_at_command(ser, 'AT+CIPPING="8.8.8.8"', 'OK')
    return success

def check_network_registration(ser):
    success, response = send_at_command(ser, "AT+CREG?", "")
    if success:
        if "+CREG: 0,1" in response or "+CREG: 0,5" in response:
            print("Successfully registered to the network.")
            return True
        else:
            print(f"Unexpected network registration status: {response}")
            return False
    else:
        print(f"Failed to check network registration: {response}")
        return False

def check_gprs_connection(ser):
    success, response = send_at_command(ser, "AT+CIPSTATUS", "STATE: IP STATUS")
    if success:
        print("GPRS connection is active.")
    else:
        print(f"Failed to verify GPRS connection: {response}")
    return success

def start_tcp_connection(ser, broker, port):
    command = f'AT+CIPSTART="TCP","{broker}","{port}"'
    success, response = send_at_command(ser, command, "CONNECT OK", timeout=10)
    if success:
        print("TCP connection started successfully.")
    else:
        print(f"Failed to start TCP connection: {response}")
    return success

def send_mqtt_packet(ser, packet):
    success, response = send_at_command(ser, "AT+CIPSEND", ">")
    if success:
        ser.write(packet)
        ser.write(b'\x1A')  # End of packet indicator
        time.sleep(2)
        response = ser.read_all().decode('utf-8').strip()
        if "SEND OK" in response:
            print("Packet sent successfully.")
        else:
            print(f"Failed to send packet: {response}")
    else:
        print(f"Failed to initiate packet send: {response}")

def generate_mqtt_connect_packet():
    packet = bytearray()
    # Fixed header
    packet.append(0x10)  # MQTT CONNECT packet type
    packet.append(0x0A)  # Remaing length 10 bytes
    # Variable header
    packet.extend(b'\x00\x04MQTT')  # Protocol name
    packet.append(0x04)  # Protocol level (4 for MQTT v3.1.1)
    packet.append(0x02)  # Connect flags (Clean session)
    packet.extend(b'\x00\x3C')  # Keep-alive timer (60 seconds)
    return bytes(packet)

def generate_mqtt_publish_packet(topic, message):
    packet = bytearray()
    packet.append(0x30)  # MQTT PUBLISH packet type
    remaining_length = 2 + len(topic) + len(message)
    packet.append(remaining_length)
    packet.append(len(topic) >> 8)
    packet.append(len(topic) & 0xFF)
    packet.extend(topic.encode('utf-8'))
    packet.extend(message.encode('utf-8'))
    return bytes(packet)

def mqtt_connect_and_publish(ser, pub_topic, pub_msg):
    packet = bytearray()
    packet.extend(generate_mqtt_connect_packet())
    packet.extend(generate_mqtt_publish_packet(pub_topic, pub_msg))
    total_length = len(packet)

    success, response = send_at_command(ser, f'AT+CIPSEND={total_length}', '>')
    if success:
        packet.extend(b'\x1A\x1A\x1A\x00')
        print(bytes(packet))
        ser.write(bytes(packet))
        ser.write(b'\x1A')
        time.sleep(2)
        response = ser.read_all().decode('utf-8').strip()
        print(f"Received: {response}")
    else:
        print('Failed to set length')

def main():
    # Replace '/dev/ttyUSB0' with the correct port for your system
    port = '/dev/ttyUSB0'
    baudrate = 9600
    timeout = 1

    try:
        ser = serial.Serial(port, baudrate, timeout=timeout)
        time.sleep(2)  # Wait for the modem to initialize

        # apn = 'INTERNET'
        apn = 'TM'
        if configure_modem(ser, apn):
            # enable_data_roaming(ser, 'TM')
            if check_network_registration(ser):
                if check_gprs_connection(ser):
                    print("Modem is configured and GPRS connection is active.")
                    if not ping(ser):
                        print('Ping failed')
                    else:
                        print('Ping success')
                    # MQTT configuration
                    broker = "194.42.111.14"
                    port = 1883
                    topic = "/wlabdb/bin"
                    message = "Hello, MQTT!"
                    if start_tcp_connection(ser, broker, port):
                        mqtt_connect_and_publish(ser, topic, message)
                        time.sleep(4)
                        response = ser.read_all().decode('utf-8').strip()
                        print("Cleaning uart buffer: %s" % response)

                        # print("Connect to broker...")
                        # mqtt_connect_packet = generate_mqtt_connect_packet()
                        # send_mqtt_packet(ser, mqtt_connect_packet)

                        # response = ser.read_all().decode('utf-8').strip()
                        # print("Cleaning uart buffer: %s" % response)
                        # if response.strip() != 'CLOSED':
                        #     print("Publishing...")
                        #     mqtt_publish_packet = generate_mqtt_publish_packet(topic, message)
                        #     send_mqtt_packet(ser, mqtt_publish_packet)
                        # else:
                        #     print('Connection CLOSED')
                        send_at_command(ser, "AT+CIPCLOSE", "OK")
                else:
                    print("Modem configuration succeeded, but failed to verify GPRS connection.")
            else:
                print("Modem configuration succeeded, but network registration failed.")
        else:
            print("Failed to configure the modem.")

        ser.close()
    except serial.SerialException as e:
        print(f"Error opening serial port {port}: {e}")

if __name__ == "__main__":
    main()
