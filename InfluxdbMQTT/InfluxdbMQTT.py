import json
import time
from paho.mqtt.client import Client as MQTTClient
from influxdb import InfluxDBClient as InfluxDB1Client

# --- Configuration MQTT ---
MQTT_BROKER = "192.168.0.2"
MQTT_PORT = 8883
MQTT_TOPIC = "#"  # souscrire à tous les topics
MQTT_CLIENT_ID = "mqtt_to_influx_secure"

# --- Configuration InfluxDB v1.8 ---
INFLUXDB_HOST = "localhost"
INFLUXDB_PORT = 8086
INFLUXDB_DB = "mqtt_data"

# Connexion InfluxDB v1
influx_client = InfluxDB1Client(
    host=INFLUXDB_HOST,
    port=INFLUXDB_PORT,
    database=INFLUXDB_DB
)

# Crée la base si elle n'existe pas
if INFLUXDB_DB not in [db['name'] for db in influx_client.get_list_database()]:
    influx_client.create_database(INFLUXDB_DB)

def on_connect(client, userdata, flags, rc):
    print("✅ Connecté au broker MQTT")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode("utf-8")
        print(f"📨 Reçu sur {msg.topic}: {payload}")
        data = json.loads(payload)

        points = []

        # Extraire les tags généraux
        device_id = data.get("device_id", "unknown")
        machine_type = data.get("type", "unknown")
        firmware = data.get("firmware", "unknown")
        uptime = data.get("uptime", "0")

        for key, content in data.items():
            if isinstance(content, dict) and "value" in content:
                try:
                    point = {
                        "measurement": key,
                        "tags": {
                            "device_id": device_id,
                            "machine_type": machine_type,
                            "firmware": firmware,
                            "unit": content.get("unit", ""),
                            "type": content.get("type", ""),
                            "topic": msg.topic
                        },
                        "fields": {
                            "value": float(content["value"])
                        },
                        "time": int(time.time() * 1_000_000_000)  # ✅ nanosecondes
                    }
                    points.append(point)
                except (ValueError, TypeError):
                    print(f"⚠️ Valeur invalide ignorée pour {key}: {content}")

        if points:
            influx_client.write_points(points)
            print(f"✅ {len(points)} point(s) écrit(s) dans InfluxDB pour {device_id}")
        else:
            print("⚠️ Aucun point valide trouvé dans le message.")

    except Exception as e:
        print(f"❌ Erreur traitement message : {e}")
        print(f"⛔ Payload brut : {msg.payload}")


# Connexion MQTT
mqtt_client = MQTTClient(MQTT_CLIENT_ID)

# Activation TLS
mqtt_client.tls_set(
    ca_certs="D:/mqtt_tls/ca.crt",        # autorité racine
)
mqtt_client.tls_insecure_set(True)
# Authentification (si login/mdp activés sur le broker)
mqtt_client.username_pw_set("espuser", "esppass")

mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
mqtt_client.loop_forever()
