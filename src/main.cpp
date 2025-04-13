#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// Identifiants Wi-Fi
const char* ssid = "telenet-8F64E";
const char* password = "a875B4KeXbcU";

// Adresse du broker (remplace par l'IP de ton PC)
const char* mqtt_server = "192.168.0.2";
const int mqtt_port = 8883;
const char* mqtt_topic = "test/topic";

const char* mqtt_user = "espuser2";
const char* mqtt_pass = "esppass";
const char* mqtt_client_id = "ESP32Client2";

// Certificat CA
const char* ca_cert = R"EOF(-----BEGIN CERTIFICATE-----
MIIDEzCCAfugAwIBAgIUJcxeaK/2OauHp4Q/Us4ylddhxx8wDQYJKoZIhvcNAQEL
BQAwGTEXMBUGA1UEAwwOTXlNb3NxdWl0dG8tQ0EwHhcNMjUwNDEyMDk0NDE2WhcN
MjYwNDEyMDk0NDE2WjAZMRcwFQYDVQQDDA5NeU1vc3F1aXR0by1DQTCCASIwDQYJ
KoZIhvcNAQEBBQADggEPADCCAQoCggEBAMpb2fbVSeR4TxAvcHh4xn3mnr/mf8OT
xcNPzPMA2C+8NO5yTxl5zUzAuDKg4vNs/FqzPy5hPr5g+NQ9faNLMRFgECE5zt86
oICavJNdJd9FtB1VFVOu/fBPdC62NZXiBtbRpi2JqqokF9NKq/aB6pQPv+1D2PHL
y2NcstKJJ1HL69z1E8qBMcJlRU/dExEkT27a/Lixr5+jw8DAbFHjgRFQtyqmoLR+
PbQiMSBSyBVldwMpjeWAvfJ9MOC7ijV3v25vnTe8ZaM8yvMCXLbEGBSeLzmiyi8y
RjLXDv5MuQ45pMRRfwgnmibvb8ZI8h6WVFJiO6vyGlpw0wOn9WIhEbMCAwEAAaNT
MFEwHQYDVR0OBBYEFDR5PLVFf/lU+dQUlKjwN1DvARveMB8GA1UdIwQYMBaAFDR5
PLVFf/lU+dQUlKjwN1DvARveMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL
BQADggEBAHV2jVTIkFiJ0XC5mXwsnSDdv4vTGXKyQ3qh1+FAKCJoZXHGCCP0m2rM
UgcMIWMTXUarU0YjBXr0u4C6SwaggkakOdQIYkH3s3PKOPE40iTZEvFYNGxQkAzJ
Z+GeQLGj+z2WVmcuvgSY6yUzvTivnbMuKJsZEjEuMv2xkl6u0SzojW/n4pSKmHjn
jhT8dSlT5CIozhT1TUHYxU5t+ViwOGyhXOCHdT0JDIP32Vnv3vnJFYGBHVAmSsqL
6hWS0879WHiDdPu7VFnSiiMFeUXkzIbYyWdTMg7aKLpJa7ZlefIXeW2hoog0MPzh
x3lCvhjzl3QrP9jgSl1Bbh1U1gtVzGQ=
-----END CERTIFICATE-----
)EOF";

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

