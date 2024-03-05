# Arduino-Smart-Door-Alarm
Arduino project developed with Ardunio Nano RP2040 Board that consists of a voice-activated smart door alarm. The alarm connects to Wi-Fi and sends messages using the MQTT protocol. For the automations I used Node-Red. There is also support for other MQTT clients integration to remotely activate/ deactivate the alarm. 

- Voice recognition -> DSpotterSDK_MakerHL the voice recognition library for Arduino Nano Rp2040, specifically designed for recognizing spoken commands and trigger words. 
- MQTT -> Is the protocol used in the project, the broker I chose is HIVEMQ. The Arduino board acts both as a publisher and a subscriber. The messages regarding the board state are sent on tht topic “arduino/alarm” and the commands received are on the topic “arduino/phone”.
- HiveMQ -> Is the broker where I configured the creditendials for all 3 clients. (Arduino board, Node-Red and my phone MQTT Client app. 
- NodeRed -> it is used as a subscriber on the topic “arduino/alarm” to detect changes made by the user with the voice or to see if the alarm was activated. It sends the user updates regarding the state of the alarm system via email.
- EasyMQTT -> Is an IOS app that acts as a MQTT client. It offers the posibility to create predefined MQTT messages and send them using the widgets on my phone’s screen. Also Siri can be configured to send commands through the app.

The file Flows.js contains the node configuration used in Node-Red and the .ino file the code that needs to be loaded on the board. To run the code you need a DSpotter licence and to configure the dependencies.
