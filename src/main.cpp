//
// Created by Roman on 30.11.2019.
//

#include <Arduino.h>

// headers for TTGO/LoRa
#include "PinsTTGO.h"
#include "LoRa.h"

// headers for OLED Display
#include "SpecsOLED.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SERIAL_BAUD 9600

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define LORA_BAND 866E6

byte loraLocalAddress = 0xB5;     // address of this device
byte loraDestination = 0xFE;      // destination to send to

unsigned long globalLoraPacketCounter = 0;
unsigned long lastSendTime = 0;

Adafruit_SSD1306 ssd1306(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RST);

void initLoRa();

void initSerial() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial) {
        delay(10);
    }
}

void initOled() {
    //reset OLED display via software
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);

    //initialize OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!ssd1306.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRRESS, false, false)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }

    ssd1306.clearDisplay();
    ssd1306.setTextColor(WHITE);
    ssd1306.setTextSize(1);
    ssd1306.setCursor(0, 0);
    ssd1306.print("LORA TTGO SX1276 TEST");
    ssd1306.display();
}

void onReceive(int packetSize) {
    if (packetSize == 0) return;          // if there's no packet, return
    // read packet header bytes:
    int recipient = LoRa.read();          // recipient address
    byte sender = LoRa.read();            // sender address
    byte incomingMsgId = LoRa.read();     // incoming msg ID
    byte incomingLength = LoRa.read();    // incoming msg length

    String incoming = "";                 // payload of packet

    while (LoRa.available()) {            // can't use readString() in callback, so
        incoming += (char) LoRa.read();      // add bytes one by one
    }

    if (incomingLength != incoming.length()) {   // check length for error
        Serial.println("error: message length does not match length");
        return;                             // skip rest of function
    }

    // if the recipient isn't this device or broadcast,
    if (recipient != loraLocalAddress && recipient != loraDestination) {
        Serial.println("This message is not for me.");
        return;
    }

    // if message is for this device, or broadcast, print details:
    Serial.println("Received from: 0x" + String(sender, HEX));
    Serial.println("Sent to: 0x" + String(recipient, HEX));
    Serial.println("Message ID: " + String(incomingMsgId));
    Serial.println("Message length: " + String(incomingLength));
    Serial.println("Message: " + incoming);
    Serial.println("RSSI: " + String(LoRa.packetRssi()));
    Serial.println("SNR: " + String(LoRa.packetSnr()));
    Serial.println();
}

void setup() {

    initSerial();
    //initOled();
    initLoRa();

}

void initLoRa() {

    // override the default CS, reset, and IRQ pins (optional)
    LoRa.setPins(TTGO_PIN_NSS, TTGO_PIN_RST, TTGO_PIN_DIO0); // set CS, reset, IRQ pin

    if (!LoRa.begin(LORA_BAND)) {         // initialize ratio at 915 MHz
        Serial.println("LoRa init failed. Check your connections.");
        while (true);                   // if failed, do nothing
    }

    // Sync Words will not receive each other's transmissions. This is one way you can filter out radios you want to ignore, without making an addressing scheme.
    //LoRa.setSyncWord(0xF3);           // ranges from 0-0xFF, default 0x34, see API docs

    // Spreading factor affects reliability of transmission at high rates, however, avoid a large spreading factor when you're sending continually.
    //LoRa.setSpreadingFactor(8);           // ranges from 6-12,default 7 see API docs

    //Serial.println("LoRa Dump Registers");
    //LoRa.dumpRegisters(Serial);

    // in receiver mode with callback (or alternatively poll in loop with receivePacket)
    LoRa.onReceive(onReceive);
    LoRa.receive();
}

void receivePacket() {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        onReceive(packetSize);
    }
}

void sendMessage(String outgoing) {
    LoRa.beginPacket();                   // start packet
    LoRa.write(loraDestination);              // add destination address
    LoRa.write(loraLocalAddress);             // add sender address
    LoRa.write(globalLoraPacketCounter);  // add message counter
    LoRa.write(outgoing.length());        // add payload length
    LoRa.print(outgoing);                 // add payload
    LoRa.endPacket();                     // finish packet and send it
    globalLoraPacketCounter++;
}

void sendPacketLoop(long waitInMillis = 500) {
    if (millis() - lastSendTime > waitInMillis) {
        String message = "Hello LoRa!";   // send a message
        sendMessage(message);
        Serial.println("Sending " + message);
        lastSendTime = millis();            // timestamp the message
        LoRa.receive();                     // go back into receive mode
    }
}

void loop() {
    // do nothing for callback or use polling send/receive
//    sendPacketLoop();
//    receivePacket();
}