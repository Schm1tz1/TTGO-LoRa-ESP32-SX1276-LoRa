//
// Created by Roman on 30.11.2019.
//

#include <Arduino.h>
#include "PinsTTGO.h"
#include "LoRa.h"

#define MAX_CONNECTION_RETRIES 50
#define LORA_BAND    868E6  //you can set band here directly,e.g. 868E6,915E6,433E6

int counter = 0;

void onReceive(int packetSize) {
    // received a packet
    if (Serial) Serial.print("Received packet '");

    // read packet
    for (int i = 0; i < packetSize; i++) {
        if (Serial) Serial.print((char) LoRa.read());
    }

    // print RSSI of packet
    if (Serial) Serial.print("' with RSSI ");
    if (Serial) Serial.println(LoRa.packetRssi());
    if (Serial) Serial.print("' and SNR ");
    if (Serial) Serial.println(LoRa.packetSnr(), 3);
}

void setup() {

    Serial.begin(9600);
    for (int i = 0; i < MAX_CONNECTION_RETRIES && !Serial; i++) {
        delay(100);
    }

    if (Serial) Serial.println("LoRa Dump Registers");

    // override the default CS, reset, and IRQ pins (optional)
    LoRa.setPins(TTGO_PIN_NSS, TTGO_PIN_RST, TTGO_PIN_DIO0); // set CS, reset, IRQ pin

    if (!LoRa.begin(LORA_BAND)) {         // initialize ratio at 915 MHz
        if (Serial) Serial.println("LoRa init failed. Check your connections.");
        while (true);                   // if failed, do nothing
    }

    LoRa.dumpRegisters(Serial);

    // in receiver mode with callback (or alternatively poll in loop with receivePacket)
    LoRa.onReceive(onReceive);
    LoRa.receive();
}

void receivePacket() {
    // try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        // received a packet
        if (Serial) Serial.print("Received packet '");

        // read packet
        while (LoRa.available()) {
            if (Serial) Serial.print((char) LoRa.read());
        }

        // print RSSI of packet
        if (Serial) Serial.print("' with RSSI ");
        if (Serial) Serial.println(LoRa.packetRssi());
        if (Serial) Serial.print("' and SNR ");
        if (Serial) Serial.println(LoRa.packetSnr(), 3);
    }
}

void sendPacket(int waitInMillis) {

    while (LoRa.beginPacket() == 0) {
        if (Serial) Serial.print("waiting for radio ... ");
        delay(waitInMillis);
    }

    if (Serial) Serial.print("Sending packet non-blocking: ");
    if (Serial) Serial.println(counter);

    // send packet
    LoRa.beginPacket();
    LoRa.print("Hello LoRa #");
    LoRa.print(counter);
    LoRa.endPacket(true);

    counter++;
}

void loop() {
    //sendPacket(100);
    //receivePacket();
}