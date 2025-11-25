# relay_subscribe_print.py
import paho.mqtt.client as mqtt
import requests
from datetime import datetime, timezone

DB_ENDPOINT = "https://kshigemotoee.com/insert_pot.php"
TOPIC_LED = "testtopic/temp/inTopic/8888"
BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC = "testtopic/temp/outTopic/8888"  # replace xxx with your unique code
CLIENT_ID = "relay-print-8888"          # unique client ID

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("[MQTT] Connected")
        client.subscribe(TOPIC, qos=0)
        client.subscribe(TOPIC_LED, qos=0)
        print(f"[MQTT] Subscribed to {TOPIC} and {TOPIC_LED}")
    else:
        print(f"[MQTT] Connect failed rc={rc}")

def on_message(client, userdata, msg):
    payload = msg.payload.decode("utf-8")
    if "," in payload:
        raw, volts = payload.split(",", 1)
        now_iso = datetime.now(timezone.utc).isoformat(timespec="seconds")
        post_data = {"raw": raw, "volts": volts, "ts": now_iso}
        resp = requests.post(DB_ENDPOINT, data=post_data)
        print(f"[MQTT] {payload} | [DB] {resp.text}")
    else:
        print(f"[MQTT] {payload}")

    if msg.topic == TOPIC_LED:
        state = payload.strip()
        now_iso = datetime.now(timezone.utc).isoformat(timespec="seconds")
        post_data = {"type": "led", "state": state, "ts": now_iso}
        resp = requests.post(DB_ENDPOINT, data=post_data)
        print(f"[MQTT LED] {state} | [DB] {resp.text}")


def main():
    client = mqtt.Client(client_id=CLIENT_ID, clean_session=True)
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(BROKER, PORT, keepalive=60)
    client.loop_forever()

if __name__ == "__main__":
    main()
