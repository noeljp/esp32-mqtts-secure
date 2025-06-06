#include <Arduino.h>
#include <WiFi.h>
extern "C" {
  #include <esp_now.h>
  #include <esp_wifi.h>
}
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

#include <MQTT.h>
#include <Ticker.h>

#include <ETH.h>

// network specific definitions
#include "Private.h"
#ifndef PRIVATE_H
#define WIFI_CHANNEL    7
#define MQTT_SERVER "192.168.1.1"
#define MQTT_USER "test"
#define MQTT_PASS "pass"
#define OTA_PASSWORD "pass"
#endif

#define LED_BUILTIN 2	// or use a NodeMCU-type of board definition
#define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT
#define ETH_POWER_PIN   12
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_ADDR        1
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18

static WiFiClient ethClient;
static MQTTClient client(512);
static Ticker updater;
static String nodeid("ethgate-");

static bool eth_connected = false;
static long lastrelayed = 0;
static long relayed = 0;

static inline void led_on() {
  digitalWrite(LED_BUILTIN, HIGH);
}
static inline void led_off() {
  digitalWrite(LED_BUILTIN, LOW);
}

static void espnow_recv(const unsigned char* mac,
  const unsigned char* data, int len
) {
    char* msg = strchr((char*)data,' ');
    if( msg == NULL ) return;
    *msg = 0;
    msg ++;
  
    led_on();

    // FIXME: we should be putting this in a queue, not sending
    // in the receive callback.
    client.publish((char*)data, msg);
    client.loop();
  
    Serial.print((char*)data);
    Serial.print(" ");
    Serial.println(msg);

    lastrelayed = millis();
    relayed ++;

    led_off();
}

static void initEspNow() {
  Serial.println("Starting ESPNOW");

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
  
  if (esp_now_init()!=0) {
    Serial.println("*** ESP_Now init failed");
    ESP.restart();
  }

  esp_now_register_recv_cb(espnow_recv);
}

void updateMessage() {
  if( !eth_connected ) return;

  char msg[256];

  snprintf(msg,sizeof(msg),
    "{\"relayed\":%d,\"idle\":%d,\"ip\":\"%s\"}",
    relayed, (millis()-lastrelayed)/1000,
    ETH.localIP().toString().c_str() );

  String topic = "sensors/";
  topic += nodeid;
  topic += "/status";

  client.publish(topic, msg);
  client.loop();

  Serial.print(topic);
  Serial.print(" ");
  Serial.println(msg);
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname(nodeid.c_str());
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;

      ArduinoOTA.begin();
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

void setup()
{
  pinMode(LED_BUILTIN,OUTPUT);

  {
    char buf[16];
    nodeid += itoa((ESP.getEfuseMac()&0xffffff000000)>>24,buf,16);
  }

  Serial.begin(115200);

  Serial.println(nodeid);

  initEspNow();

  WiFi.onEvent(WiFiEvent);
  ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN,
	  ETH_TYPE, ETH_CLK_MODE);

  client.begin(MQTT_SERVER, 1883, ethClient);
  client.loop();

  ArduinoOTA.setHostname(nodeid.c_str());
  ArduinoOTA.setPassword(OTA_PASSWORD);
  
  MDNS.begin(nodeid.c_str());

  updater.attach(60, updateMessage);
}

static long last_connect = 0;

void loop() {
  if( !client.connected() && millis()-last_connect>5000 ) {
    Serial.println("(Re)connecting to MQTT");
    client.connect(nodeid.c_str(),MQTT_USER,MQTT_PASS);
    last_connect = millis();
  }

  ArduinoOTA.handle();
  client.loop();

  delay(1);
}
