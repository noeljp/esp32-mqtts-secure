#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "secrets.h" 


const char* mqtt_topic = "test/topic";


// MQTT client
WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connexion à ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connecté au WiFi !");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());
  Serial.print("Adresse server : ");
  Serial.println(mqtt_server);
  Serial.print("Port : ");
  Serial.println(mqtt_port);
}

void reconnect() {
  // Tentative de reconnexion en boucle
  while (!client.connected()) {
    Serial.print("Connexion au broker MQTT...");
    if (client.connect("ESP32Client2", mqtt_user, mqtt_pass)) {
      Serial.println("connecté !");
    } else {
        Serial.print("échec, rc=");
        Serial.print(client.state());
        Serial.println(" nouvelle tentative dans 5 secondes");
        // Attendre 5 secondes avant de réessayer
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  // Charger le certificat
  espClient.setCACert(ca_cert);
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    const char* msg = "Hello depuis ESP32 (MQTTS)";
    Serial.print("Envoi: ");
    Serial.println(msg);
    client.publish(mqtt_topic, msg);
  }
}

