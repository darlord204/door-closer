# door-closer

This is a work in progress.

Although not much of the original code exists, great inspiration came from  https://github.com/thehookup/Motorized_MQTT_Blinds. Thanks to The Hook Up https://www.youtube.com/channel/UC2gyzKcHbYfqoXA5xbyGXtQ/about

Closing a door with a swinging arm requires a surprising amount of torque. I ended up using 3 stages of this
planetary gear box https://www.thingiverse.com/thing:2586962.

I rewrote the code using this library https://github.com/plapointe6/EspMQTTClient.  It includes a lot of the
code normally needed which made the sketch a lot simpler.  It also allows for OTA updates through a wed uploader by going to the devices IP address or mDNS name.  Use the user name and password from the "EspMQTTClient client" section of the sketch to log in. The firmware can be uploaded using the .bin file produced by "Export Compiled Binary" in the Arduino IDE's "Sketch" menu.


## File setup

Fill out the entire USER CONFIGURATION section of the code.


## Home Assistant YAML


Replace "doorCloser" with your MQTT_CLIENT_ID if you changed it in the file setup

```yaml
switch:
  - platform: mqtt
  name: "Door Closer"
  state_topic: "door_closer/state"
  command_topic: "door_closer/steps"
  retain: false
  # Number of steps needed to close the door
  payload_on: "10000"
  payload_off: "0"

    input_number:
      stepper_control:
        name: "Stepper Control"
        initial: 0
        min: -1000
        max: 1000
        step: 1
        mode: slider

  ```
  Automations.YAML file
```yaml
  - id: '1588397267850'
  alias: Adjust Door Closer
  description: ''
  trigger:
  - entity_id: input_number.stepper_control
    platform: state
  condition: []
  action:
  - data:
      payload_template: '{{states.input_number.stepper_control.state |int }}'
      topic: door_closer/stepFine
    service: mqtt.publish

```
