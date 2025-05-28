#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


// MIKIEDIT BASIC ENCRYPT FUNCTS
uint8_t encryptionLookup[256];
uint8_t decryptionLookup[256];

void build_lookup_tables(uint16_t key) {
  for (uint16_t i = 0; i < 256; i++) {
    uint8_t encrypted = i ^ ((key >> (i % 16)) | (key << (16 - (i % 16))));
    encryptionLookup[i] = encrypted;
    decryptionLookup[encrypted] = i;
  }
}

void encrypt_message(const char* input, char* output, size_t len) {
  for (size_t i = 0; i < len; i++) {
    output[i] = encryptionLookup[(uint8_t)input[i]];
  }
}
void decrypt_message(const char* input, char* output, size_t len) {
  for (size_t i = 0; i < len; i++) {
    output[i] = decryptionLookup[(uint8_t)input[i]];
  }
}

build_lookup_tables(0xABCD);  

RF24 radio(0, 1); // CE, CSN

const byte txAddr[6] = "NodeA";  // Send to Node B
const byte rxAddr[6] = "NodeB";  // Listen as Node A

// USE FOR OTHER TEENSY
//const byte txAddr[6] = "NodeB";  // Send to Node A
//const byte rxAddr[6] = "NodeA";  // Listen as Node B

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
    String msg = Serial.readStringUntil('\n');
    if(msg)
    char encrypted[32];
    encrypt_message(msg.c_str(), encrypted, msg.length() + 1);
    radio.openWritingPipe(txAddr);
    radio.write(encrypted, msg.length() + 1);
    //radio.write(msg.c_str(), msg.length() + 1);
    radio.startListening();

  }

  // Read from RF
  if (radio.available()) {
    //radio.read(&buffer, sizeof(buffer));
    //Serial.println(buffer);
    
    radio.read(&buffer, sizeof(buffer));
    char decrypted[32];
    decrypt_message(buffer, decrypted, strlen(buffer));
    Serial.println(decrypted);
  }
}


