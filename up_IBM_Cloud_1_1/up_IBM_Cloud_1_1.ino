//To upload data to IBM Cloud
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

//Device configuration for IBM Cloud
#define ORG "337hw2" 
#define DEVICE_TYPE "NodeMCU" 
#define DEVICE_ID "483FDAAA63A1" 
#define TOKEN "GGACU4fQHZVsugUXsJ"

//mq3,buzzer,led pin setup
int buzzer = D2;
int sensor = A0;
int led = D5;

//For MQ3 sensor calibration
double m = -0.75489;
double b = 1.99646;

float ppm_limit = 200;

//WIFI Credentials
const char* ssid = "DV3546";
const char* password = "heartofgold";

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char pubTopic1[] = "iot-2/evt/Sensor_data_1/fmt/json";
char pubTopic2[] = "iot-2/evt/Sensor_data_2/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

WiFiClient wifiClient;
PubSubClient client(server, 1883, NULL, wifiClient);

void setup() {
  // put your setup code here, to run once:
      pinMode(buzzer, OUTPUT);
      pinMode(sensor, INPUT);
      pinMode(led, OUTPUT);

      Serial.begin(115200);
      delay(10);
      Serial.println("Connecting to ");
      Serial.println(ssid);
 
 
      WiFi.begin(ssid, password);
 
      while (WiFi.status() != WL_CONNECTED)
     {
          delay(500);
          Serial.print(".");
     }
      Serial.println("");
      Serial.println("WiFi connected");

      if (!client.connected()) 
     {
          Serial.println(server);
          while (!client.connect(clientId, authMethod, token)) 
         {
              Serial.print(".");
              delay(500);
         }
          Serial.println("Bluemix connected");
     }
}

long lastMsg = 0;
float sv;
float voltage;

void loop() {
  // put your main code here, to run repeatedly:
      float sensor_value = analogRead(sensor);
      sv = (float) sensor_value;
      voltage = sensor_voltage(sv);

      Serial.print("Gas Level: ");
      Serial.println(sensor_value);

      Serial.print("Sensor Voltage: ");
      Serial.println(voltage);


      float RS_gas; //  Get the value of RS via in a clear air
      float ratio;
 
/*-----------------------------------------------*/
 
      RS_gas = (5.0 - voltage)/voltage; // omit *RL
      //R0 = RS_air/60.0; // The ratio of RS/R0 is 60 in a clear air from Graph

      ratio = RS_gas/0.01301;  // ratio = RS/R0 
  /*-----------------------------------------------------------------------*/
 
      Serial.print("RS_ratio = ");
      Serial.println(RS_gas);
      Serial.print("Rs/R0 = ");
      Serial.println(ratio);
 
      Serial.print("\n\n");
 
      delay(1000);
  
      double ppm_log = (log10(ratio)-b)/m; //Get ppm value in linear scale according to the the ratio value 
      double ppm = pow(10, ppm_log); //Convert ppm value to log scale 
      double percentage = ppm/10000; //Convert to percentage 
      Serial.print(percentage); //Load screen buffer with percentage value 
      Serial.print("%"); //Load screen buffer with "%"
      //Serial.display(); //Flush characters to screen
 
      Serial.print("sensor voltage = ");
      Serial.print(voltage);
      Serial.println("V");
 
      Serial.print("Ratio = ");
      Serial.println(ratio);
      delay(1000);
      //R0 = 0.01142 approx
      
      if (ppm > ppm_limit)
      {
          digitalWrite(led, HIGH);
          digitalWrite(buzzer, HIGH);
          delay(500);
          digitalWrite(buzzer, LOW);
      }
      else
      {
          digitalWrite(buzzer, LOW);
          digitalWrite(led, LOW);
      }

      client.loop();
      long now = millis();
      if (now - lastMsg > 3000) 
      {
          lastMsg = now;
          String payload1 = "{\"d\":{\"Name\":\"" DEVICE_ID "\"";
          payload1 += ",\"Gas concentration\":";
          payload1 += ppm;
          payload1 += "}}";
          Serial.print("Sending payload: ");
          Serial.println(payload1);

          if (client.publish(pubTopic1, (char*) payload1.c_str()))
          {
              Serial.println("Publish ok");
          } 
          else 
          {
              Serial.println("Publish failed");
          }

          String payload2 = "{\"d\":{\"Name\":\"" DEVICE_ID "\"";
          payload2 += ",\"Percentage concentration\":";
          payload2 += percentage;
          payload2 += "}}";
          Serial.print("Sending payload1: ");
          Serial.println(payload2);

          if (client.publish(pubTopic2, (char*) payload2.c_str()))
          {
              Serial.println("Publish ok");
          } 
          else 
          {
              Serial.println("Publish failed");
          }
      }
      delay(1000);
}

//Function for calculating Voltage of MQ3 sensor
float sensor_voltage(float s_in){
      float v;
      
      v = (s_in/1024.0)*5.0;
      
      return v;
}
