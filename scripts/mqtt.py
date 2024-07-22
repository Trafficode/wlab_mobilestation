import paho.mqtt.client as mqtt
import struct

# Define the MQTT broker details
broker = "194.42.111.14"
port = 1883
topic = "/wlabdb/bin"

def wlab_bin2dict(packet):
    packet_dict = {
        "version": packet[0],
        "id": "%02X:%02X:%02X:%02X:%02X:%02X" % (packet[1], packet[2], packet[3], packet[4], packet[5], packet[6]),
        "ts": struct.unpack('<q', packet[7:15])[0],
        "temp_act": struct.unpack('<h', packet[15:17])[0],
        "temp_avg": struct.unpack('<h', packet[17:19])[0],
        "temp_max": struct.unpack('<h', packet[19:21])[0],
        "temp_min": struct.unpack('<h', packet[21:23])[0],
        "temp_max_ts_offset": struct.unpack('<h', packet[23:25])[0],
        "temp_min_ts_offset": struct.unpack('<h', packet[25:27])[0],
        "humidity_act": packet[27],
        "humidity_avg": packet[28],
        "humidity_max": packet[29],
        "humidity_min": packet[30],
        "humidity_max_ts_offset": struct.unpack('<h', packet[31:33])[0],
        "humidity_min_ts_offset": struct.unpack('<h', packet[33:35])[0],
        "battery_voltage": struct.unpack('<h', packet[35:37])[0],
    }
    return(packet_dict)

def wlab_bin2old(packet_dict):
    packet_wlab_dict = {
        "UID": packet_dict["id"],
        "TS": packet_dict["ts"],
        "SERIE": {
            "Temperature": {
                "f_avg": "%.1f" % (packet_dict["temp_avg"]/10),
                "f_act": "%.1f" % (packet_dict["temp_act"]/10),
                "f_min": "%.1f" % (packet_dict["temp_min"]/10),
                "f_max": "%.1f" % (packet_dict["temp_max"]/10),
                "i_min_ts": packet_dict["ts"] + packet_dict["temp_min_ts_offset"],
                "i_max_ts": packet_dict["ts"] + packet_dict["temp_max_ts_offset"],
            },
            "Humidity": {
                "f_avg": "%.1f" % (packet_dict["temp_avg"]/10),
                "f_act": "%.1f" % (packet_dict["temp_act"]/10),
                "f_min": "%.1f" % (packet_dict["temp_min"]/10),
                "f_max": "%.1f" % (packet_dict["temp_max"]/10),
                "i_min_ts": packet_dict["ts"] + packet_dict["humidity_min_ts_offset"],
                "i_max_ts": packet_dict["ts"] + packet_dict["humidity_max_ts_offset"],
            }
        }
    }
    return(packet_wlab_dict)

# Callback function when a message is received
def on_message(client, userdata, message):
    # print(f"Received message: {message.payload.decode()} on topic {message.topic}")
    _dict = wlab_bin2dict(message.payload)
    print(_dict)
    wlab_old = wlab_bin2old(_dict)
    print(wlab_old)

# Callback function when connected to the broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected successfully to the broker")
        client.subscribe(topic)  # Subscribe to the topic
        pub_packet = bytearray()
        pub_packet.extend(b'\x01\x10\x21\x32\xa0\xfa\x03\x50\xd4\x12\x00\x00\x00\x00\x00')
        pub_packet.extend(b'\x96\x00\x7c\x00\x44\x01\x20\xff\xe0\x00\x18\x00\x21\x00\x2d')
        pub_packet.extend(b'\x0f\x55\x20\x00\x4d\x01\x70\x01')
        client.publish(topic, pub_packet)
    else:
        print(f"Connect failed with code {rc}")

# Callback function when a subscription is confirmed
def on_subscribe(client, userdata, mid, granted_qos):
    print(f"Subscribed to topic with QoS: {granted_qos[0]}")

# Create an MQTT client instance
client = mqtt.Client()

# Attach the callback functions to the client
client.on_message = on_message
client.on_connect = on_connect
client.on_subscribe = on_subscribe

# Connect to the broker
client.connect(broker, port)

# Start the network loop
client.loop_forever()