#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(0, 1); // CE, CSN

//const byte txAddr[6] = "NodeA";  // Send to Node B
//const byte rxAddr[6] = "NodeB";  // Listen as Node A

// USE FOR OTHER TEENSY
const byte txAddr[6] = "NodeB";  // Send to Node A
const byte rxAddr[6] = "NodeA";  // Listen as Node B

void setup() {
  Serial.begin(115200);
  while (!Serial);

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(100);
  radio.setRetries(3, 5);
  radio.enableDynamicPayloads();
  radio.setAutoAck(true);
  radio.openWritingPipe(txAddr);
  radio.openReadingPipe(1, rxAddr);
  radio.startListening();
}

char buffer[32];

void loop() {
  // Send from Serial to RF
  if (Serial.available()) {
    radio.stopListening();
    String msg = Serial.readStringUntil('\0');
    if(msg)
    radio.openWritingPipe(txAddr);
    radio.write(msg.c_str(), msg.length() + 1);
    radio.startListening();

  }

  // Read from RF
  if (radio.available()) {
    radio.read(&buffer, sizeof(buffer));
    Serial.println(buffer);
  }
}
