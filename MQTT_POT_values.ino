#include <ESP8266WiFi.h>
#include <PubSubClient.h>
const char* ssid = "Roja";
const char* password = "12345679";
const char* mqtt_server = "91.121.93.94";
const char* mqtt_topic = "potentiometer";
const int potentiometerPin = A0;
WiFiClient espClient;
PubSubClient client(espClient);
void setup_wifi()
{
  delay(10);
 // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup()
{
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  int potValue = analogRead(potentiometerPin);
  Serial.print("Potentiometer Value: ");
  Serial.println(potValue);
  char payload[10];
  snprintf(payload, sizeof(payload), "%d", potValue);
  if (client.publish(mqtt_topic, payload))
  {
    Serial.println("Message sent to MQTT server.");
  }
  else
  {
    Serial.println("Error sending message to MQTT server.");
  }
  delay(1000);
}