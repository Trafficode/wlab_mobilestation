import paho.mqtt.client as mqtt
import sys 
import json
import time

try:
    ConfigFile = sys.argv[1]
    config_f = open(ConfigFile, 'r')
    ConfigData = json.load(config_f)
    config_f.close()
    # print("ConfigData: %s" % json.dumps(ConfigData, indent=4))
except:
    print("File %s, not found or json is not correct" % ConfigFile)

# Callback function when a message is received
def on_message(client, userdata, message):
    print(f"Received message: {message.payload.decode()} on topic {message.topic}")

# Callback function when connected to the broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected successfully to the broker")
        client.publish(ConfigData["mqtt"]["auth_topic"], json.dumps(ConfigData["auth"]))
        print("Authenticated...")
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
client.connect(ConfigData["mqtt"]["broker"], ConfigData["mqtt"]["port"])

# Start the network loop
client.loop_forever()