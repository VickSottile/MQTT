/* Clase ECO2
   Ejemplo ESP32 + MQTT + DS18B20
   Publica temperatura en broker MQTT
*/

#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ---------- CONFIGURACIÓN WiFi ----------
const char* ssid = "Personal-AA1-2.4GHz";
const char* password = "17EFB6CAA1";

// ---------- CONFIGURACIÓN MQTT ----------
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* tempTopic = "esim/temperatura";  // Tema donde se publicará la temperatura

// ---------- SENSOR DS18B20 ----------
const int oneWireBus = 4;  // Pin de datos del sensor (puede cambiarse)
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// ---------- OBJETO MQTT ----------
AsyncMqttClient mqttClient;

// ---------- FUNCIONES ----------
void setupWifi() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Conectado a WiFi");
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("✅ Conectado al servidor MQTT");
}

// Si se desconecta, reintenta cada 5 segundos
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("❌ Desconectado del servidor MQTT. Reintentando...");
  delay(5000);
  mqttClient.connect();
}

void publicarTemperatura() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC != DEVICE_DISCONNECTED_C) {
    char tempString[8];
    dtostrf(tempC, 4, 2, tempString);
    mqttClient.publish(tempTopic, 1, false, tempString);
    Serial.print("🌡️ Temperatura publicada: ");
    Serial.println(tempString);
  } else {
    Serial.println("⚠️ Error: sensor no detectado");
  }
}

void setup() {
  Serial.begin(115200);

  setupWifi();

  sensors.begin();
  Serial.println("Sensor DS18B20 inicializado");

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.connect();
}

unsigned long lastPublish = 0;

void loop() {
  // Publicar cada 10 segundos
  if (millis() - lastPublish > 10000) {
    lastPublish = millis();
    publicarTemperatura();
  }
}
