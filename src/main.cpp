#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//#include <OneWire.h>
//#include <DallasTemperature.h >

#include "DHT.h" 
#define DHT_TYPE DHT22 
const int DHT_PIN = 3; 
DHT dht(DHT_PIN, DHT_TYPE);



const int sleepSeconds = 10;
const char *ssid = "rheinsberg.mesh"; //WLAN Netzwerk
const char *password = "1234567899";  //WLAN Passwort

const char *hostname = "MQTT-Thermometer";
//const char* mqtt_server = "duemchen.feste-ip.net";int mport = 56686;
const char *mqtt_server = "192.168.10.51";
int mport = 1883;
const char *mqtt_pathV = "simago/fussboden/innentemp";
const char *mqtt_pathH = "simago/fussboden/feuchte";

const char *mqtt_user = "th433";

// Data wire is connected to GPIO13  D7
//#define ONE_WIRE_BUS 13

WiFiClient wifiClient;
PubSubClient client(wifiClient);
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

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



//-----------------------------------------------------------------------------------------------------------------------------------------------

 
// runs over and over again 

 




void setup()
{
  delay(300);
  Serial.begin(115200);
  while (!Serial)
  {
    delay(30);
  }
  
  Serial.println("\n\nMQTT-Thermometer");
  WiFi.disconnect();
  delay(300);
  Serial.println("Starting...");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  client.setServer(mqtt_server, 1883);

  Serial.println("Temperature DHT22");
  // Start up the library  
  //pinMode(1, FUNCTION_3);
  pinMode(3, FUNCTION_3);
  dht.begin(); 
      
}

void loop()
{
  unsigned long endtime = 0;
  while (true) {
    if (!client.connected())
    {
      reconnect();
    }
    client.loop();
    Serial.print("Requesting temperatures...");
    float h = dht.readHumidity(); 
    float t = dht.readTemperature(); 
    if(isnan(t)) {
      delay(1000);
      continue;
    }
      
    //h=100.1;    t=-12.23;
    Serial.print("Temperatur: "); 
    Serial.print(t); 
    Serial.print("°C, Luftfeuchtigkeit: "); 
    Serial.print(h); 
    Serial.println("%");     
    Serial.print("Publish message: ");
    float tempV = t;    
    char str_temp[6];
    dtostrf(tempV, 4, 2, str_temp);
    sprintf(msg, "%s", str_temp);
    Serial.print(mqtt_pathV);
    Serial.print("  ");
    Serial.print(msg);
    Serial.print("  \n");
    client.publish(mqtt_pathV, msg);
    Serial.print("Publish message: ");
    tempV = h;    
    dtostrf(tempV, 4, 2, str_temp);
    sprintf(msg, "%s", str_temp);
    Serial.print(mqtt_pathH);
    Serial.print("  ");
    Serial.print(msg);
    Serial.print("  \n");
    client.publish(mqtt_pathH, msg);       
    endtime = millis()+ 1 * 1000;
    //  sec warten auf MQTT-Sendung
    delay(1000);
    ESP.deepSleep(1000000*6);
  }
}
