import paho.mqtt.client as mqtt
import struct
import datetime
import pytz
import time
import threading
import json

# Define the MQTT broker details
broker = "194.42.111.14"
port = 1883
topic = "/wlabdash/pixel"

# Create an MQTT client instance
client = mqtt.Client()

def tid_proc(name, count):
    for i in range(count):
        print(f"Thread {name} is running: {i}")
        client.publish("/wlabdash/pixel", json.dumps({
            "temp": [
                {"timestamp": 1634066895000, "val": 12},
                {"timestamp": 1634066895010, "val": 14}
            ]
        }))

        time.sleep(1.0)
    print("tid_proc done")

# Create and start a thread with arguments
tid = threading.Thread(target=tid_proc, args=("tid_proc", 5))

# Callback function when a message is received
def on_message(client, userdata, message):
    print(f"Received message: {message.payload.decode()} on topic {message.topic}")

# Callback function when connected to the broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected successfully to the broker")
        client.subscribe(topic)  # Subscribe to the topic
    else:
        print(f"Connect failed with code {rc}")

# Callback function when a subscription is confirmed
def on_subscribe(client, userdata, mid, granted_qos):
    print(f"Subscribed to topic with QoS: {granted_qos[0]}")
    tid.start()

# Attach the callback functions to the client
client.on_message = on_message
client.on_connect = on_connect
client.on_subscribe = on_subscribe

# Connect to the broker
client.connect(broker, port)

# Start the network loop
client.loop_forever()
