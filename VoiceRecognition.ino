#include <Arduino.h>
#include <DSpotterSDK_MakerHL.h>
#include <LED_Control.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <stdio.h>
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include "arduino_secrets.h"
// The DSpotter License Data.
#include "CybLicense_1704716443.h"
#define DSPOTTER_LICENSE g_lpdwLicense

// The DSpotter Keyword Model Data.
#if defined(TARGET_ARDUINO_NANO33BLE) || defined(TARGET_PORTENTA_H7) || defined(TARGET_NICLA_VISION)
// For ARDUINO_NANO33BLE and PORTENTA_H7
#include "Model_L1.h"             // The packed level one model file.
// For NANO_RP2040_CONNECT
#elif defined(TARGET_NANO_RP2040_CONNECT)
#include "Model_1704716443.h"             // The packed level zero model file.
#endif
#define DSPOTTER_MODEL g_lpdwModel

// The VR engine object. Only can exist one, otherwise not worked.
static DSpotterSDKHL g_oDSpotterSDKHL;
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the WiFi radio's status

WiFiSSLClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "e9fd258abea04a858e2bd98c2356066b.s1.eu.hivemq.cloud";
int        port     = 8883;
const char topic1[]  = "arduino/alarm";
const char topic2[] = "arduino/phone";


#include <Arduino_LSM6DSOX.h>
#include <MadgwickAHRS.h>

Madgwick MF;
#define M_PI   3.14159265358979323846264338327950288



void setup_wifi() {
  delay(10);
  Serial.println();
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LEDR, HIGH);
    delay(500);
    digitalWrite(LEDR, LOW);
    WiFi.begin(SECRET_SSID, SECRET_PASS);
  }
}


void setup_MQTT() {
  mqttClient.setUsernamePassword(SECRET_MQTT_USERNAME, SECRET_MQTT_PASSWORD);
  // Serial.print("Attempting to connect to the MQTT broker: ");
  // Serial.println(broker);

  while (!mqttClient.connect(broker, port)) {
    delay(100);
  }
  mqttClient.subscribe(topic2);

  // Serial.println("You're connected to the MQTT broker!");
  // Serial.println();
}


void turnOnTheAlarm()
{
  LED_BUILTIN_Off();
  mqttClient.beginMessage(topic1);       //send the confirmation to the broker 1 = alarm ON
  mqttClient.print("1\n");
  mqttClient.endMessage();

  for(int i=0;i<10;i++)
  {
    digitalWrite(LEDG, HIGH);
    delay(500);
    digitalWrite(LEDG, LOW);
    delay(500);
  }
  float acc, initial;
  initial = getDoorAcceleration();
  while(1)
  {
    acc = getDoorAcceleration();
    if(initial-acc > 0.05 || initial - acc < -0.05)
    {
      mqttClient.beginMessage(topic1);       //send the confirmation to the broker code 3 = BREACH alert
      mqttClient.print("3\n");
      mqttClient.endMessage();
      for(int i=0;i<20; i++)         //blink the blue led 10 times fast to visually show that the door has been breached
      {
        digitalWrite(LEDB, HIGH);
        delay(100);
        digitalWrite(LEDB, LOW);
        digitalWrite(LEDR, HIGH);
        delay(100);
        digitalWrite(LEDR, LOW);
      }
    }
    char msg = getPhoneMessage();
    if(msg=='2')
    {
      shutDown("4\n");
      break;
    }
  }
}
void shutDown(const char* msg)         //2='voice comand deactivated' 5='Alarm deactivated'
{
  LED_BUILTIN_Off();
  for(int i=0;i<3;i++)
  {
    digitalWrite(LEDR, HIGH);
    delay(500);
    digitalWrite(LEDR, LOW);
    delay(500);
  }
  mqttClient.beginMessage(topic1);
  mqttClient.print(msg);
  mqttClient.endMessage();
  if(msg=="2\n")
  {
    while (1)
    {
      char msg = getPhoneMessage();
      if(msg=='1')
      {
        turnOnTheAlarm();
      }
      if(msg=='3')                          //shut down phone command mode
      {
        mqttClient.beginMessage(topic1);
        mqttClient.print("5\n");
        mqttClient.endMessage();
        for(int i=0;i<3;i++)
        {
          digitalWrite(LEDG, HIGH);
          delay(500);
          digitalWrite(LEDG, LOW);
          delay(500);
        }
        break;
      }
    }
  }
}

void VRCallback(int nFlag, int nID, int nScore, int nSG, int nEnergy)
{
  if (nFlag==DSpotterSDKHL::GetResult)
  {
      
      switch(nID)
      {
        case 10000:               //turn on the alarm voice message   "turn on the alarm"
          turnOnTheAlarm();
          break;
        case 10002:               //shut down the alarm voice message  "shut down"
          shutDown("2\n");        //2 = 'Voice comand was deactivated'
          break;
        default:
          break;
      }
  }
  else if (nFlag==DSpotterSDKHL::ChangeStage)
  {
      switch(nID)
      {
          case DSpotterSDKHL::TriggerStage:
            LED_RGB_Off();
            LED_BUILTIN_Off();
            break;
          case DSpotterSDKHL::CommandStage:
            LED_BUILTIN_On();
            break;
          default:
            break;
      }
  }
}

void setup_IMU()
{
  if (!IMU.begin()) {
    // Serial.println("Failed to initialize IMU!");
    while (1);
  }
  MF.begin(104.00);
  // Serial.println("Initialized IMU!");
}

float getDoorAcceleration()
{
  float Ax, Ay,Az;
  if (IMU.accelerationAvailable()) {
    //reading the data that we need
   IMU.readAcceleration(Ax, Ay, Az);
  }
  // Serial.println(Az);
  delay(100);
  return Az;
}
char getPhoneMessage()
{
  char message = '0';
  int messageSize = mqttClient.parseMessage();
  if (messageSize) 
  {
    // we received a message, print out the topic and contents
    message = (char)mqttClient.read();    
  }
  return message;         //0 = no message 1 = activate 2= deactivate 3 = turn back on voice command 
}



void setup()
{
  LED_Init_All();
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  // Init Serial output for show debug info
  // Serial.begin(9600);
  // while(!Serial);
  DSpotterSDKHL::ShowDebugInfo(false);
  MF.begin(104.00);

  setup_wifi();
  digitalWrite(LEDG, HIGH);
  delay(500);
  digitalWrite(LEDG, LOW);
  delay(500);
  setup_MQTT();
  digitalWrite(LEDB, HIGH);
  delay(500);
  digitalWrite(LEDB, LOW);
  delay(500);
  setup_IMU();
  // Init VR engine & Audio
  if (g_oDSpotterSDKHL.Init(DSPOTTER_LICENSE, sizeof(DSPOTTER_LICENSE), DSPOTTER_MODEL, VRCallback) != DSpotterSDKHL::Success)
    return;
  mqttClient.beginMessage(topic1);
  mqttClient.print("6\n");
  mqttClient.endMessage();
}

void loop()
{  
  g_oDSpotterSDKHL.DoVR();
}



