#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


// MIKIEDIT BASIC ENCRYPT FUNCTS
uint8_t encryptionLookup[256];
uint8_t decryptionLookup[256];

uint8_t encrypt_byte(uint8_t val, uint16_t key) {
  return val ^ ((key >> (val % 16)) | (key << (16 - (val % 16))));
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

void build_lookup_tables(uint16_t key) {
  bool used[256] = {false};

  for (uint16_t i = 0; i < 256; i++) {
    if (i == '\n') {
      encryptionLookup[i] = i;
      used[i] = true;
      continue;
    }

    uint8_t enc = encrypt_byte(i, key);
    
    // Ensure we don't map anything into '\n' or already used value
    while (enc == '\n' || used[enc]) {
      enc = (enc + 1) % 256;
    }

    encryptionLookup[i] = enc;
    used[enc] = true;
  }

  // Build reverse lookup
  for (uint16_t i = 0; i < 256; i++) {
    decryptionLookup[encryptionLookup[i]] = i;
  }
}



RF24 radio(0, 1); // CE, CSN

//const byte txAddr[6] = "NodeA";  // Send to Node B
//const byte rxAddr[6] = "NodeB";  // Listen as Node A

// USE FOR OTHER TEENSY
const byte txAddr[6] = "NodeB";  // Send to Node A
const byte rxAddr[6] = "NodeA";  // Listen as Node B

void setup() {
  Serial.begin(115200);
  while (!Serial);
  unsigned long before = micros();
  build_lookup_tables(0xABCD); 
  

//  String msg = "Miki Korol\n"
  //char encrypted[32];
  //encrypt_message(msg.c_str(), encrypted, msg.length()+1);
  
  unsigned long after = micros();
  unsigned long diff = after-before;
  Serial.print(diff);
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
  bool encryptEnabled = digitalRead(23);  
  
  if (Serial.available()) {
    radio.stopListening();
    String msg = Serial.readStringUntil('\n');
    if (msg.length() > 0) {
      radio.openWritingPipe(txAddr);
      if (encryptEnabled) {
        char encrypted[32];
        encrypt_message(msg.c_str(), encrypted, msg.length()+1);//removed +1 after msg length to not encrypt null
        radio.write(encrypted, msg.length() + 1);
      } 
      else {
        radio.write(msg.c_str(), msg.length() + 1);
      }
    }
    radio.startListening();
  }

  if (radio.available()) {
    radio.read(&buffer, sizeof(buffer));
    if (encryptEnabled) {
      char decrypted[32];
      decrypt_message(buffer, decrypted, strlen(buffer));
      Serial.println(decrypted);
    } 
    else {
      Serial.println(buffer);
    }
  }
}



