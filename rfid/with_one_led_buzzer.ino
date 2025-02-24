#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 7
#define LED_PIN 3    // External LED
#define SPEAKER_PIN 4// Speaker/buzzer

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

void setup() { 
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(LED_PIN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT); // Initialize speaker pin
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // Activate both LED and speaker
  digitalWrite(LED_PIN, HIGH);
  tone(SPEAKER_PIN, 1000); // Generate 1kHz tone
  
  Serial.print(F("RFID Detected - UID:"));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  // Keep active for 500ms
  delay(500);
  
  // Turn off both devices
  digitalWrite(LED_PIN, LOW);
  noTone(SPEAKER_PIN);

  rfid.PICC_HaltA();
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}