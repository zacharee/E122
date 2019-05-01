/*
 E122 - Baseline Wemos Firmware - Version 0.3-
 Updated: April 1 2019

 Integrated from components picked off the net by Prof.KP. 
 This version serves as starting point for integration of Low Power Mode


********/
/*  Installation of drivers and other set up needed - Check CANVAS...

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/


/*
 * Publishing URL
 * http://dmi.stevens.edu/mqtt/getdata.php?macid=XXXX
 */


#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include "info.h"
#include "WiFiManager.h"
#include "DHT.h"
#include "WeMosSleep.h"

  //The ESP8266 recognizes different pins than what is labelled on the WeMos D1 
  #if defined(d1)  //Defines Wemos D1 R1 pins to GPIO pins
    #define D0 3
    #define D1 1
    #define D2 16
    #define D8 0
    #define D9 2
    #define D5 14
    #define D6 12
    #define D7 13
    #define D10 15
  #endif 
  #if defined(d1_mini) //Defines Wemos D1 R2 pins to GPIO pins
    #define A0 0
    #define D0 16
    #define D1 5
    #define D2 4
    #define D3 0
    #define D4 2
    #define D5 14
    #define D6 12
    #define D7 13
    #define D8 15
  #endif

//Set up the DHT11 (temperature/humidity sensor)
#define DHTPIN D6  //PLUG THE DHT 11 ONLY INTO D5, D6, D7 on either D1-R1 or D1-R2 Board. 
                   // DO NOT PLUG INTO OTHERS AS THE MAPPING IS NOT THE SAME. DHT11 WILL BURN. 
#define DHTTYPE DHT11


// CONFIGURATION SETTINGS ....BEGIN

//Wifi Settings
//const char* ssid = "DLabsPrivate1";
//const char* password = "L3tsM@keSometh1n";
const char* ssid = "Stevens-Media";
const char* password = "Stevens1870";


//MQTT Settings
const char* mqtt_server = "155.246.18.226";
const char* MQusername = "jojo";
const char* MQpassword = "hereboy";

//MQTT Publish Topics
//XXXX will automatically be replaced by last 4 digits of MAC address
char* MQtopic1 = "E122/XXXX/Temperature";
char* MQtopic2 = "E122/XXXX/Humidity";
char* MQtopic3 = "E122/XXXX/Light";
//Note that since this is a real Wemos board -- it runs forever as opposed to 
// the fakemos -- http://www.dmi.stevens.edu/fakemos/


//create class instance for DHT-11 temperature/humidity sensor
DHT dht(DHTPIN,DHTTYPE);

//create class instance for WiFi connection
WiFiClient espClient;

//create class instance to retrieve MAC address
info board_info;

//create class instance for MQTT Client
PubSubClient client(espClient);

WeMosSleep sleep;

//declare variables
char msg1[20],msg2[20],msg3[20]; //value strings for temperature, humidity and light
float temp, hum, light; //values for temperature, humidity and light

//controls connections
bool connectedToWiFi = false;
bool connectedToMqtt = false;

int InvalidCtr = 0;

void readSensors();

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to "); Serial.println(ssid);

  //connect to WiFi
  WiFi.begin(ssid, password);
  //output a "." every 500ms until connection established
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  connectedToWiFi = true;
  randomSeed(micros());  //insure random call later on is random

  Serial.println(""); Serial.println("WiFi connected");
  //output IP address assigned by WiFi access point/router
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
}

//If a MQTT topic is subscribed to this function will be invoked when a topic
// is received from the MQTT server.
//For now the LED can be turned on and off
void callback(char* topic, byte* payload, unsigned int length) {
  //print topic and payload(value)
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

void mqttConnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect to MQTT server
    if (client.connect(clientId.c_str(),MQusername,MQpassword)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish(MQtopic1, "00000");
      // ... and resubscribe ---- Dont subscribe KP
      // client.subscribe("inTopic");
      // #KP - No announments --- No Subscribes. 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  connectedToMqtt = true;
  
}


//replace XXXX in MQTT topic with last 4 digits of MAC address
void insertMAC(char* Topic){
int posX = 0;
  int strLen = strlen(Topic);  //get length of topic

  //find first X in topic string, ie "E122/XXXX/Temperature"
  for(int i=0; i<strLen; i++){
    if(Topic[i]=='X'){posX = i; break;}
  }
  if(posX > 0){  //if 'X' found
    //replace XXXX with last 4 digits of MAC address
    Topic[posX]=board_info.mac()[12];   //00:00:00:00:x0:00
    Topic[posX+1]=board_info.mac()[13]; //00:00:00:00:0x:00
    Topic[posX+2]=board_info.mac()[15]; //00:00:00:00:00:x0
    Topic[posX+3]=board_info.mac()[16]; //00:00:00:00:00:0x
  }
}


void setup() {
  pinMode(BUILTIN_LED, OUTPUT);

  Serial.begin(115200);

  while (!Serial){}

  Serial.println("");
  Serial.println("Wemos POWERING UP ......... ");

  sleep.setNapSeconds(12);
  sleep.setSleepMinutes(15);
  sleep.checkWake();
  
  Serial.print("Mac Address:"); Serial.println(board_info.mac());

  insertMAC(MQtopic1);
  insertMAC(MQtopic2);
  insertMAC(MQtopic3);

  if(!connectedToWiFi){
    setup_wifi();
  }

  dht.begin();

  delay(100);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

//code in this function runs repeatedly
void loop() 
{   //start loop

  if(!connectedToMqtt)
    {
    //check if connected to MQTT server, if not try to reconnect
    if (!client.connected()) 
    {
      mqttConnect();
    }
    //run MQTT tasks
    client.loop();
    }

  readSensors();  //read temperature, humidity and light sensors

  sleep.sleep();
}  //end loop

void readSensors() {
    //read temperature from DHT-11 sensor, true = return Farenheit
    temp = dht.readTemperature(true);
    //read humidity from DHT-11 sensor
    hum = dht.readHumidity();

    //if(temp > 1000) InvalidCtr++;
    //Serial.print("InvalidCtr="); Serial.println(InvalidCtr);

    //read light sensor at A0 connection, value will be 0 to 1023
    //(float) is a typecast to convert the returned integer to a float
    light = (float) analogRead(A0);

    //convert 0 to 1023 value to voltage
    light = light/310;  //ADC value max / Vmax = 1024/3.3 = 310

    //convert values to strings
    // 20 specifies max string size
    // %d specifies an integer (non-fraction) conversion
    // (int) is a "type cast" to convert a float to an integer
    snprintf (msg1, 20, "%d", (int) temp);
    snprintf (msg2, 20, "%d", (int) hum);
    snprintf (msg3, 20, "%5.3f", light);  //5.3 for "0.000" format

    //output MQTT Topic and Message to serial monitor for diagnostics
    Serial.print("Published :" ); Serial.print(MQtopic1);
    Serial.print(" with value: " ); Serial.println(msg1);
    client.publish(MQtopic1, msg1);  //publish temperature to MQTT server

    Serial.print("Published :" ); Serial.print(MQtopic2);
    Serial.print(" with value: " ); Serial.println(msg2);
    client.publish(MQtopic2, msg2);  //publish humidity to MQTT server

    Serial.print("Published :" ); Serial.print(MQtopic3);
    Serial.print(" with value: " ); Serial.println(msg3);
    client.publish(MQtopic3, msg3);  //publish light voltage to MQTT server
    Serial.println("");

    delay(2000);  //wait to publish data again

}
