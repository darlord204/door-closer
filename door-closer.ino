//
// Publish the amount of steps to close the door to topic "step"
//     This will cause to motor to turn in one direction and then reverse the same amount

// Publish the amount of steps to fine tune to topic "stepFine" as either a + or - number
//      to move forward and backward if the arm gets out of place
//
// Change SSID, WPA2 Password, USER_MQTT_CLIENT_NAME and mqtt_server
//
//

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <AH_EasyDriver.h>
#include <PubSubClient.h>

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "WPA2 Password"
#endif
#define USER_MQTT_PORT            1883

#define USER_MQTT_CLIENT_NAME     "Device Name Here"         // Used to define MQTT topics, MQTT Client ID, and ArduinoOTA

#define STEPPER_SPEED             150                  //Defines the speed in RPM for your stepper motor
#define STEPPER_STEPS_PER_REV     1028                //Defines the number of pulses that is required for the stepper to rotate 360 degrees
#define STEPPER_MICROSTEPPING     0                   //Defines microstepping 0 = no microstepping, 1 = 1/2 stepping, 2 = 1/4 stepping
#define DRIVER_INVERTED_SLEEP     1                   //Defines sleep while pin high.  If your motor will not rotate freely when on boot, comment this line out.

#define STEPPER_DIR_PIN           5
#define STEPPER_STEP_PIN          4
#define STEPPER_SLEEP_PIN         0
#define STEPPER_MICROSTEP_1_PIN   15
#define STEPPER_MICROSTEP_2_PIN   14


//Global Variables
bool boot = true;
int currentPosition = 0;
int newPosition = 0;
char positionPublish[50];
bool moving = false;
char charPayload[50];
int intPayload = 0;

const char* ssid = STASSID;
const char* password = STAPSK;
const char* mqtt_server = "192.168.xxx.xxx";

AH_EasyDriver shadeStepper(STEPPER_STEPS_PER_REV, STEPPER_DIR_PIN , STEPPER_STEP_PIN, STEPPER_MICROSTEP_1_PIN, STEPPER_MICROSTEP_2_PIN, STEPPER_SLEEP_PIN);

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  String newTopic = topic;
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';
  String newPayload = String((char *)payload);
  intPayload = newPayload.toInt();
  Serial.println(newPayload);
  Serial.println();
  // newPosition = intPayload;
  Serial.print("intPayload [");
  Serial.print(intPayload);
  Serial.print("] ");
  Serial.println();
  DashedLine();

  if (newTopic == "step-test-fine")
  {
    Serial.print("Received fine tunning");
    Serial.println();
    DashedLine();
    processStepperFine();
  }
  else
  {
    processStepper();
  }
}


void DashedLine()
{
  Serial.println("----------------");
}
void processStepperFine()
{
  Serial.println();
  Serial.print("In processStepperFine");
  Serial.println();
  Serial.print("inPayload is =");
  Serial.print(intPayload);
  Serial.println();

  if (intPayload >= 1){
      for (int i = 0; i <= intPayload; i++) {
      shadeStepper.move(1, FORWARD);
      yield();
    }
  }
  else
  {
  for (int i = 0; i >= intPayload; i--) {
      shadeStepper.move(1, BACKWARD);
      yield();
    }
  //yield(); //needed to stop reboot
  }
}
void processStepper()
{
  Serial.println();
  Serial.print("In processStepper");
  Serial.println();
  Serial.print("inPayload is =");
  Serial.print(intPayload);
  Serial.println();


    for (int i = 0; i <= intPayload; i++) {
      shadeStepper.move(1, FORWARD);
      yield();
    }
  delay(1000);
    for (int i = 0; i <= intPayload; i++) {
      shadeStepper.move(1, BACKWARD);
      yield();
    }


}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      //once connected to MQTT broker, subscribe command if any
      client.subscribe("step");
      client.subscribe("stepFine");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 6 seconds before retrying
      delay(6000);
    }
  }
} //end reconnect()

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(STEPPER_STEP_PIN, OUTPUT);
  pinMode(STEPPER_DIR_PIN, OUTPUT);
  pinMode(STEPPER_SLEEP_PIN, OUTPUT);
  // pinMode (actPin, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();
}
