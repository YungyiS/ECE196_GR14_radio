#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
}
}
//hardcoded symetric key encryption function
void T1encrypt(uint16_t* ADCOutput, size_t length, uint16_t key) {
  for (size_t i = 0; i < length; i++) {
    ADCOutput[i] ^= (key >> (i % 16)) | (key << (16 - (i % 16)));
  }
}

void T1Decrypt(uint16_t* TransmittedData, size_t length, uint16_t key) {
  for (size_t i = 0; i < length; i++) {
    TransmittedData[i] ^= (key >> (i % 16)) | (key << (16 - (i % 16)));
  }
}
