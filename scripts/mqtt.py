import paho.mqtt.client as mqtt

# Define the MQTT broker details
broker = "194.42.111.14"
port = 1883
topic = "/wlabdb/bin"

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