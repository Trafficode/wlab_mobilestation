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



packet_bytes = bytearray()
packet_bytes.extend(generate_mqtt_connect_packet())
packet_bytes.extend(b'\x1A')
connect_packet_hex = ''
for b in packet_bytes:
    connect_packet_hex += '%02X ' % b

print('CONNECT')
print('AT+CIPSEND')
print(connect_packet_hex)

packet_bytes = bytearray()
packet_bytes.extend(generate_mqtt_publish_packet('/wlabdb/bin', '123'))
packet_bytes.extend(b'\x1A')
pub_packet_hex = ''
for b in packet_bytes:
   pub_packet_hex += '%02X ' % b

print('PUBLISH')
print('AT+CIPSEND')
print(pub_packet_hex)

# success, response = send_at_command(ser, f'AT+CIPSEND={total_length}', '>')
# if success:
#     packet.extend(b'\x1A\x1A\x1A\x00')
#     print(bytes(packet))
#     ser.write(bytes(packet))
#     ser.write(b'\x1A')
#     time.sleep(2)
#     response = ser.read_all().decode('utf-8').strip()
#     print(f"Received: {response}")
# else:
#     print('Failed to set length')