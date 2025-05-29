#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


// MIKIEDIT BASIC ENCRYPT FUNCTS

/*
//WORKING
char base = 32;
char range = 95;

char encrypt_char(char c, uint16_t key) {
  // Encrypt to printable ASCII (32–126)
  return base + ((c ^ (key & 0xFF)) % range);
}

void encrypt_string(const String& input, String& output, uint16_t key) {
  output = "";
  for (size_t i = 0; i < input.length(); i++) {
    output += encrypt_char(input[i], key);
  }
}

char decrypt_char(char c, uint16_t key) {
  // Brute force reverse (only works if mapping is simple)
  for (char guess = 0; guess < 127; guess++) {
    if (encrypt_char(guess, key) == c) return guess;
  }
  return '?';
}

void decrypt_string(const String& input, String& output, uint16_t key) {
  output = "";
  for (size_t i = 0; i < input.length(); i++) {
    output += decrypt_char(input[i], key);
  }
}*/


char encryptionLookup[95];   // Maps: ASCII 32–126 (95 printable chars)
char decryptionLookup[95];   // Reverse lookup

void build_lookup_tables(uint16_t key) {
  bool used[95] = {false};

  for (int i = 0; i < 95; ++i) {
    char plain = i + 32;  // printable char
    char enc = 32 + ((plain ^ (key & 0xFF)) % 95);

    // Ensure unique and printable
    while (used[enc - 32]) {
      enc = 32 + ((enc - 31) % 95);  // wrap in printable range
    }

    encryptionLookup[i] = enc;
    decryptionLookup[enc - 32] = plain;
    used[enc - 32] = true;
  }
}

void encrypt_string(const String& input, String& output) {
  output = "";
  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i];
    if (c >= 32 && c <= 126) {
      output += encryptionLookup[c - 32];
    } else {
      output += c;  // Leave non-printables unchanged
    }
  }
}

void decrypt_string(const String& input, String& output) {
  output = "";
  for (size_t i = 0; i < input.length(); i++) {
    char c = input[i];
    if (c >= 32 && c <= 126) {
      output += decryptionLookup[c - 32];
    } else {
      output += c;  // Leave non-printables unchanged
    }
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

  uint16_t key = 0xABCD;
  build_lookup_tables(key);


  pinMode(23,INPUT);
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
uint16_t key = 0xABCD;
void loop() {
  bool encryptEnabled = digitalRead(23);  
  
  if (Serial.available()) {
    radio.stopListening();
    String msg = Serial.readStringUntil('\0');
    if (msg.length() > 0) {
      radio.openWritingPipe(txAddr);
      if (encryptEnabled) {
        String encrypted;
        encrypt_string(msg,encrypted);    //FOR LUT
        
        //encrypt_string(msg, encrypted, key);

        radio.write(encrypted.c_str(), msg.length() + 1);
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
      String buffer_str = String(buffer);
      String decrypted;
      decrypt_string(buffer_str, decrypted);
      //decrypt_message(buffer, decrypted, strlen(buffer));
      Serial.println(decrypted);
    } 
    else {
      Serial.println(buffer);
    }
  }
}



