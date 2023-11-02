#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>
const char* ssid = "Airtel_2g";
const char* password = "airdelta@2g";
const char* mqtt_server = "91.121.93.94";
const int mqtt_port = 1883;
const char* mqtt_topic ="#RFID";
WiFiClient espClient;
PubSubClient client(espClient);
void setup() 
{
  Serial.begin(9600); // Set the baud rate to match Arduino UNO
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
   delay(1000);
   Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) 
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client")) 
    {
      Serial.println("Connected to MQTT");
      client.subscribe(mqtt_topic);
    } 
    else
    {
      Serial.print("MQTT Connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}
void loop()
{
  if (!client.connected())
  {
   reconnect();
  }
  while(Serial.available())
  {
    //client.publish(mqtt_topic,"no data");
    String message = Serial.readStringUntil('\n');
    client.publish(mqtt_topic, message.c_str());
    Serial.println("Sent to MQTT: " + message);
  }
   client.loop();
}

void reconnect()
{
  while (!client.connected())
  {
    //Serial.println("Reconnecting to MQTT...");
    if (client.connect("ESP8266Client"))
    {
      Serial.println("Connected to MQTT");
      client.subscribe(mqtt_topic);
    }
    else
    {
      Serial.print("MQTT Connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}