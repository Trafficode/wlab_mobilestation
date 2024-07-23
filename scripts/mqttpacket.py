def generate_mqtt_connect_packet(client_id):
    packet = bytearray()
    # Fixed header
    packet.append(0x10)  # MQTT CONNECT packet type
    packet.append(12 + len(client_id))  # Remaing length 10 bytes
    # Variable header
    packet.extend(b'\x00\x04MQTT')  # Protocol name
    packet.append(0x04)  # Protocol level (4 for MQTT v3.1.1)
    packet.append(0x02)  # Connect flags (Clean session)
    packet.extend(b'\x00\x3C')  # Keep-alive timer (60 seconds)
    packet.append(0x00)
    packet.append(len(client_id))
    packet.extend(client_id.encode('utf-8'))
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
packet_bytes.extend(generate_mqtt_connect_packet('DIGI'))
packet_bytes.extend(b'\x1A')
connect_packet_hex = ''
for b in packet_bytes:
    connect_packet_hex += '%02X ' % b

print('CONNECT')
print('AT+CIPSEND')
print(connect_packet_hex)

packet_bytes = bytearray()
packet_bytes.extend(generate_mqtt_publish_packet('/wlabdb/test', 'Test...'))
packet_bytes.extend(b'\x1A')
pub_packet_hex = ''
for b in packet_bytes:
   pub_packet_hex += '%02X ' % b

print('PUBLISH')
print('AT+CIPSEND')
print(pub_packet_hex)