#include <Arduino.h>
#include <stdint.h>

#define KEY_LENGTH 16 // 16 photons, 2 bits each

typedef struct {
  uint8_t data[(KEY_LENGTH + 7) / 8];
} Basis;

typedef struct {
  uint8_t data[(KEY_LENGTH + 3) / 4];
} Key;

Basis senderBasis;
Basis measurementBasis;
Key senderKey;
Key measuredKey;

uint8_t get_basis_value(const Basis* b, int idx) {
  int byte_index = idx / 8;
  int bit_offset = 7 - (idx % 8);
  return (b->data[byte_index] >> bit_offset) & 0x1;
}

void set_basis(Basis* b, int idx, uint8_t value) {
  int byte_index = idx / 8;
  int bit_offset = 7 - (idx % 8);
  b->data[byte_index] &= ~(1 << bit_offset);
  b->data[byte_index] |= (value & 0x1) << bit_offset;
}

void fill_random_basis_bytes(Basis* b) {
  for (int i = 0; i < sizeof(b->data); i++) {
    b->data[i] = (uint8_t)random(0, 256);
  }
}

void display_basis(const Basis* b) {
  Serial.println("1-bit basis values:");
  for (int i = 0; i < KEY_LENGTH; i++) {
    Serial.print(get_basis_value(b, i));
    if (i < KEY_LENGTH - 1) Serial.print(", ");
  }
  Serial.println();
}

void display_key(const Key* k) {
  Serial.println("Key (2-bit photons):");
  for (int i = 0; i < KEY_LENGTH; ++i) {
    int byte_index = i / 4;
    int bit_offset = (3 - (i % 4)) * 2;
    uint8_t photon = (k->data[byte_index] >> bit_offset) & 0x3;
    Serial.print((photon >> 1) & 1);
    Serial.print(photon & 1);
    if (i < KEY_LENGTH - 1) Serial.print(", ");
  }
  Serial.println();
}

void generate_random_photons_and_basis(Key* key, Basis* basis) {
  for (int i = 0; i < KEY_LENGTH; ++i) {
    int byte_index = i / 4;
    int bit_offset = (3 - (i % 4)) * 2;

    uint8_t b = random(0, 2);
    set_basis(basis, i, b);

    uint8_t photon;
    if (b == 0) {
      photon = random(0, 2) ? 0b01 : 0b10;
    } else {
      photon = random(0, 2) ? 0b00 : 0b11;
    }

    key->data[byte_index] &= ~(0x3 << bit_offset);
    key->data[byte_index] |= (photon << bit_offset);
  }
}

Key measure_recieved_key(const Key* received, const Basis* receiverBasis) {
  Key measured = {0};

  for (int i = 0; i < KEY_LENGTH; ++i) {
    int byte_index = i / 4;
    int bit_offset = (3 - (i % 4)) * 2;
    uint8_t photon = (received->data[byte_index] >> bit_offset) & 0x3;

    uint8_t sender_basis = !((photon >> 1) ^ (photon & 1)); // XNOR
    set_basis(&senderBasis, i, sender_basis);
    uint8_t receiver_basis = get_basis_value(receiverBasis, i);

    uint8_t new_photon;
    if (sender_basis == receiver_basis) {
      new_photon = photon;
    } else {
      new_photon = (receiver_basis == 0) ?
                   (random(0, 2) ? 0b01 : 0b10) :
                   (random(0, 2) ? 0b00 : 0b11);
    }

    measured.data[byte_index] &= ~(0x3 << bit_offset);
    measured.data[byte_index] |= (new_photon << bit_offset);
  }

  return measured;
}

Key set_real_key(const Key* measuredKey, const Basis* myBasis, const Basis* receivedBasis) {
  Key realKey = {0};

  for (int i = 0; i < KEY_LENGTH; ++i) {
    int byte_index = i / 4;
    int bit_offset = (3 - (i % 4)) * 2;

    uint8_t my_basis = get_basis_value(myBasis, i);
    uint8_t their_basis = get_basis_value(receivedBasis, i);

    uint8_t photon;
    if (my_basis == their_basis) {
      photon = (measuredKey->data[byte_index] >> bit_offset) & 0x3;
    } else {
      photon = 0b00;
    }

    realKey.data[byte_index] &= ~(0x3 << bit_offset);
    realKey.data[byte_index] |= (photon << bit_offset);
  }

  return realKey;
}

uint8_t encryptionLookup[256];

void build_encryption_table(uint16_t key) {
  for (uint16_t i = 0; i < 256; i++) {
    encryptionLookup[i] = encrypt_byte(i, key);
  }
}

uint8_t encrypt_byte(uint8_t val, uint16_t key) {
  return val ^ ((key >> (val % 16)) | (key << (16 - (val % 16))));
}


void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for USB Serial

  fill_random_basis_bytes(&measurementBasis);
  Serial.println("Receiver basis:");
  display_basis(&measurementBasis);

  generate_random_photons_and_basis(&senderKey, &senderBasis);

  Serial.println("Sender's basis:");
  display_basis(&senderBasis);

  Serial.println("Sent key:");
  display_key(&senderKey);

  measuredKey = measure_recieved_key(&senderKey, &measurementBasis);

  Serial.println("Measured key:");
  display_key(&measuredKey);

  Key realKey = set_real_key(&measuredKey, &measurementBasis, &senderBasis);

  Serial.println("Final key (basis matched only):");
  display_key(&realKey);
}

void loop() {

}
