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
const int temperatureHigh = 33;
const int criticalTemperature = 40;

// Soil moisture sensor
const int moisturePin = A1;

// Fan motor
const int fan = 47;

// LED as alert
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
  // Initialize serial communication at a baud rate of 115200
  Serial.begin(115200);

  // Initialize the DHT sensor for temperature and humidity readings
  dht.begin();

  // Set the fan pin as an OUTPUT to control the fan motor
  pinMode(fan, OUTPUT);

  // Set the red LED pin as an OUTPUT for visual indication
  pinMode(redLED, OUTPUT);

  // Turn off the red LED initially to signify a normal state
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
  float moisture = analogRead(moisturePin); 

  // Variable to store the condition of the fan motor ('On' or 'Off')
  char motor[4];

  // Variable to store the condition of the LED ('On' or 'Off')
  char led[4];

  // Handle fan motor condition to regulate temperature
  if (temperature > temperatureHigh)
  {
    strcpy(motor, "On");
  }
  else
  {
    strcpy(motor, "Off");
  }

  // Handle LED condition when the temperature detected exceeds or equals to the critical temperature
  if (temperature >= criticalTemperature)
  {
    strcpy(led, "On");
  }
  else
  {
    strcpy(led, "Off");
  }

  // Size of expected output
  char payload[300]; 

  // Format payload string
  sprintf(payload, "Temperature: %.2f, Humidity: %.2f, Moisture: %.2f, Motor: %s, Red LED Light: %s", temperature, humidity, moisture, motor, led);

  // Publish the formatted telemetry data to the MQTT topic
  client.publish(MQTT_TOPIC, payload); 
}
