//
// Publish the amount of steps to close the door to topic "step"
//     This will cause to motor to turn in one direction and then reverse the same amount

// Publish the amount of steps to fine tune to topic "stepFine" as either a + or - number
//      to move forward and backward if the arm gets out of place
//
//
//
//

#include "EspMQTTClient.h"
#include <AH_EasyDriver.h>

#define USER_MQTT_CLIENT_NAME     "door_closer"
#define CLOSER_MOVE_TOPIC         "steps"
#define FINE_TUNE_TOPIC           "stepFine"

#define STEPPER_SPEED             150                  //Defines the speed in RPM for your stepper motor
#define STEPPER_STEPS_PER_REV     1028                //Defines the number of pulses that is required for the stepper to rotate 360 degrees
#define STEPPER_MICROSTEPPING     0                   //Defines microstepping 0 = no microstepping, 1 = 1/2 stepping, 2 = 1/4 stepping
#define DRIVER_INVERTED_SLEEP     1                   //Defines sleep while pin high.  If your motor will not rotate freely when on boot, comment this line out.

#define STEPPER_DIR_PIN           5
#define STEPPER_STEP_PIN          4
#define STEPPER_SLEEP_PIN         0
#define STEPPER_MICROSTEP_1_PIN   15
#define STEPPER_MICROSTEP_2_PIN   14

AH_EasyDriver stepper(STEPPER_STEPS_PER_REV, STEPPER_DIR_PIN , STEPPER_STEP_PIN, STEPPER_MICROSTEP_1_PIN, STEPPER_MICROSTEP_2_PIN, STEPPER_SLEEP_PIN);


EspMQTTClient client(
  "SSID",
  "wifi-password",
  "192.168.0.60",  // MQTT Broker server ip
  "admin",   // Can be omitted if not needed
  "admin",   // Can be omitted if not needed
  USER_MQTT_CLIENT_NAME,     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

// Move the stepper forward (+ value) or backward (- value) the number of steps
//spesified in the FINE_TUNE_TOPIC topic.
void processStepperFine(const String & payload)
{
  // Convert the String to int so we can do some math on it
  int intPayload = payload.toInt();


  Serial.println();
  Serial.print("In processStepperFine");
  Serial.println();
  Serial.print("inPayload is =");
  Serial.print(intPayload);
  Serial.println();

  if (intPayload >= 1){
      for (int i = 0; i <= intPayload; i++) {
      stepper.move(1, FORWARD);
      yield();
    }
  }
  else
  {
  for (int i = 0; i >= intPayload; i--) {
      stepper.move(1, BACKWARD);
      yield();
    }
  }
}

// Move forward the amount of steps in the "CLOSER_MOVE_TOPIC" topic and then back the same amount

void processStepper(const String & payload)
{
  int intpayload2 = payload.toInt();
  Serial.println();
  Serial.print("In processStepper");
  Serial.println();
  Serial.print("inPayload2 is =");
  Serial.print(intpayload2);
  Serial.println();


    for (int i = 0; i <= intpayload2; i++) {
      stepper.move(1, FORWARD);
      yield();
    }
  delay(1000);
    for (int i = 0; i <= intpayload2; i++) {
      stepper.move(1, BACKWARD);
      yield();
    }
}

void setup()
{
  Serial.begin(115200);

  // Optionnal functionnalities of EspMQTTClient :
  //client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage(USER_MQTT_CLIENT_NAME"/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  // Subscribe to "mytopic/test" and display received message to Serial
  //client.subscribe(USER_MQTT_CLIENT_NAME"/"CLOSER_MOVE_TOPIC, [](const String & payload) {});

  client.subscribe(USER_MQTT_CLIENT_NAME"/"CLOSER_MOVE_TOPIC, processStepper);
  client.subscribe(USER_MQTT_CLIENT_NAME"/"FINE_TUNE_TOPIC, processStepperFine);

  // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
  //client.subscribe("mytopic/wildcardtest/#", [](const String & topic, const String & payload) {
  //  Serial.println(topic + ": " + payload);
  // });

  // Publish a message to "mytopic/test"
  //client.publish("mytopic/test", "This is a message"); // You can activate the retain flag by setting the third parameter to true

  // Execute delayed instructions
  //client.executeDelayed(5 * 1000, []() {
   // client.publish("mytopic/test", "This is a message sent 5 seconds later")});
}

void loop()
{
  client.loop();
}
