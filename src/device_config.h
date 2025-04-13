#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads1;
Adafruit_ADS1115 ads2;

// Déclarations globales
const char* site_id = "technopole";
const char* apu_id = "mydata";
const char* device_id = "ARM1"; // À ajuster selon la machine
const char* machine_type = "armoire"; // "armoire", "four", "frigo", etc.
const char* mqtt_subscribe_topic = "test/cmd";
const char* mqtt_publish_topic = "test/topic";
const char* firmware_version = "1.0.0";

// Pins I2C personnalisés (à adapter selon ton matériel)
#define I2C_SDA 21
#define I2C_SCL 22

// Pins PWM et entrées digitales
#define PWM_1 25
#define PWM_2 26
#define DIGI_IN_1 32
#define DIGI_IN_2 33

// Variables internes pour les mesures
float pt100_1 = 0.0;
float pt100_2 = 0.0;
float humidity = 0.0;
float pressure = 0.0;
float rms_1 = 0.0;
float rms_2 = 0.0;
float sensor_1 = 0.0;
float sensor_2 = 0.0;
bool digital_in_1 = false;
bool digital_in_2 = false;

// Fonction de lecture depuis ADS1115 avec loi de conversion (exemple générique)
float readADCChannel(int ch) {
    int16_t adc_value;
    if (ch < 4){
        //adc_value = ads1.readADC_SingleEnded(ch);
        adc_value = 1724; // Valeur fictive pour le test
    } else {
        //adc_value = ads2.readADC_SingleEnded(ch-4);
        adc_value = 1724; // Valeur fictive pour le test
    }

  float voltage = adc_value * 0.1875 / 1000.0; // gain = 1
  return voltage;
}

// Fonction de mise à jour des données capteurs selon le type
void updateSensors() {
  if (strcmp(machine_type, "armoire") == 0) {
    pt100_1 = (readADCChannel(0) - 0.5) * 100.0;
    pt100_2 = (readADCChannel(1) - 0.5) * 100.0;
    humidity = readADCChannel(2) * 100.0;
    pressure = readADCChannel(3) * 10.0;
    rms_1 = readADCChannel(4) * 230.0;
    rms_2 = readADCChannel(5) * 230.0;
    sensor_1 = readADCChannel(6);
    sensor_2 = readADCChannel(7);
    digital_in_1 = digitalRead(DIGI_IN_1);
    digital_in_2 = digitalRead(DIGI_IN_2);
  }
}

// Initialise les capteurs/IO en fonction du type de machine
void initMachine() {
  if (strcmp(machine_type, "armoire") == 0) {
    Serial.println("Initialisation capteurs armoire");
    Wire.begin(I2C_SDA, I2C_SCL);
    
    //ads1.begin(0x48);
    //ads2.begin(0x49);

    pinMode(DIGI_IN_1, INPUT);
    pinMode(DIGI_IN_2, INPUT);
    pinMode(PWM_1, OUTPUT);
    pinMode(PWM_2, OUTPUT);
  } else if (strcmp(machine_type, "four") == 0) {
    Serial.println("Initialisation four");
  } else if (strcmp(machine_type, "frigo") == 0) {
    Serial.println("Initialisation frigo");
  }
}

// Retourne le statut global de la machine (à adapter dynamiquement)
const char* getStatus() {
  return "OK";
}
void fillMachineData(JsonDocument& doc) {
    updateSensors();
  
    if (strcmp(machine_type, "armoire") == 0) {
      doc["pt100_1"]["value"] = pt100_1;
      doc["pt100_1"]["unit"] = "°C";
      doc["pt100_1"]["type"] = "temperature";
  
      doc["pt100_2"]["value"] = pt100_2;
      doc["pt100_2"]["unit"] = "°C";
      doc["pt100_2"]["type"] = "temperature";
  
      doc["humidity"]["value"] = humidity;
      doc["humidity"]["unit"] = "%";
      doc["humidity"]["type"] = "humidity";
  
      doc["pressure"]["value"] = pressure;
      doc["pressure"]["unit"] = "hPa";
      doc["pressure"]["type"] = "pressure";
  
      doc["rms_1"]["value"] = rms_1;
      doc["rms_1"]["unit"] = "V";
      doc["rms_1"]["type"] = "voltage";
  
      doc["rms_2"]["value"] = rms_2;
      doc["rms_2"]["unit"] = "V";
      doc["rms_2"]["type"] = "voltage";
  
      doc["sensor_1"]["value"] = sensor_1;
      doc["sensor_1"]["unit"] = "V";
      doc["sensor_1"]["type"] = "analog_input";
  
      doc["sensor_2"]["value"] = sensor_2;
      doc["sensor_2"]["unit"] = "V";
      doc["sensor_2"]["type"] = "analog_input";
  
      doc["digital_in_1"]["value"] = digital_in_1;
      doc["digital_in_1"]["unit"] = "bool";
      doc["digital_in_1"]["type"] = "digital_input";
  
      doc["digital_in_2"]["value"] = digital_in_2;
      doc["digital_in_2"]["unit"] = "bool";
      doc["digital_in_2"]["type"] = "digital_input";
    }
  
    else if (strcmp(machine_type, "four") == 0) {
      doc["setpoint_c"]["value"] = 120.0;
      doc["setpoint_c"]["unit"] = "°C";
      doc["setpoint_c"]["type"] = "temperature_setpoint";
  
      doc["temperature_c"]["value"] = 118.7;
      doc["temperature_c"]["unit"] = "°C";
      doc["temperature_c"]["type"] = "temperature";
  
      doc["heating_on"]["value"] = true;
      doc["heating_on"]["unit"] = "bool";
      doc["heating_on"]["type"] = "state";
  
      doc["overheat_alarm"]["value"] = false;
      doc["overheat_alarm"]["unit"] = "bool";
      doc["overheat_alarm"]["type"] = "alarm";
    }
  
    else if (strcmp(machine_type, "frigo") == 0) {
      doc["temperature_c"]["value"] = 4.2;
      doc["temperature_c"]["unit"] = "°C";
      doc["temperature_c"]["type"] = "temperature";
  
      doc["door_open"]["value"] = false;
      doc["door_open"]["unit"] = "bool";
      doc["door_open"]["type"] = "door_status";
  
      doc["compressor_running"]["value"] = true;
      doc["compressor_running"]["unit"] = "bool";
      doc["compressor_running"]["type"] = "motor_status";
  
      doc["defrost_cycle"]["value"] = false;
      doc["defrost_cycle"]["unit"] = "bool";
      doc["defrost_cycle"]["type"] = "cycle_status";
  
      doc["alarms"] = JsonArray();  // ou une vraie liste si dispo
    }
  }
  
