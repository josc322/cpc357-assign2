// ESP32 publish telemetry data into Google Cloud
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// Setup MQTT
const char *WIFI_SSID = "abc@unifi";        // WiFi SSID
const char *WIFI_PASSWORD = "abc";       // WiFi password
const char *MQTT_SERVER = "34.132.64.84";    // MQTT server address
const int MQTT_PORT = 1883;                   // MQTT server port
const char *MQTT_TOPIC = "iot";               // MQTT topic

// DHT22 sensor
#define DHT_PIN 40
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// Temperature threshold
const int TemperatureThreshold = 30;

// Soil moisture sensor
const int moisturePin = A1;
const int MinMoistureValue = 4095;
const int MaxMoistureValue = 2060;
const int MinMoisture = 0;
const int MaxMoisture = 100;
const int MoistureThreshold = 30;

// Fan motor
const int fan = 47;

// LED as alert for soil moisture
const int redLED = 39;

// WiFi 
WiFiClient espClient;
PubSubClient client(espClient);

// Function to setup WiFi connection
void setup_wifi() 
{
  delay(10);
  Serial.println("Connecting to WiFi..."); 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("."); 
  }
  
  Serial.println("WiFi connected"); 
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(fan, OUTPUT);
  pinMode(redLED, OUTPUT);
  digitalWrite(redLED, LOW);
  // Setup WiFi connection
  setup_wifi(); 
  // Set the MQTT server and port
  client.setServer(MQTT_SERVER, MQTT_PORT);
}

// Function to reconnect to MQTT broker
void reconnect() 
{
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection..."); 
    if (client.connect("ESP32Client"))
    {
      Serial.println("Connected to MQTT server"); 
    }
    else
    {
      Serial.print("Failed, rc="); Serial.print(client.state()); 
      Serial.println(" Retrying in 5 seconds..."); 
      delay(5000);
    } 
  }
}

void loop() 
{
  // Ensure MQTT connection is maintained
  if (!client.connected()) 
  {
    reconnect(); 
  }
  
  // Process incoming MQTT messages and handle keep-alive
  client.loop();

  // Delay for 5 seconds
  delay(5000); 

  // Read DHT22 results
  // Read temperature
  float temperature = dht.readTemperature();

  // Read humidity
  float humidity = dht.readHumidity(); 

  // Read moisture level results
  float moistureLevel = analogRead(moisturePin); 
  float moisture = map(moistureLevel, MinMoistureValue, MaxMoistureValue, MinMoisture, MaxMoisture);

  char motor[4];
  char led[4];

  // Handle fan motor condition if temperature is too high
  if (temperature >= TemperatureThreshold)
  {
    strcpy(motor, "On");
    digitalWrite(fan, HIGH);
  }
  else
  {
    strcpy(motor, "Off");
    digitalWrite(fan, LOW);
  }

  // Handle LED condition if moisture level is too low
  if (moisture <= MoistureThreshold)
  {
    strcpy(led, "On");
    digitalWrite(redLED, HIGH);
  }
  else
  {
    strcpy(led, "Off");
    digitalWrite(redLED, LOW);
  }

  // Size of expected output
  char payload[300]; 

  // Format payload string
  sprintf(payload, "Temperature: %.2f, Humidity: %.2f, Moisture: %.2f, Motor: %s, Red LED Light: %s", temperature, humidity, moisture, motor, led);

  // Publish the formatted telemetry data to the MQTT topic
  client.publish(MQTT_TOPIC, payload); 
}

