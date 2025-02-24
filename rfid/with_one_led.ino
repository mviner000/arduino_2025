#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 7
#define LED_PIN 3 // External LED connected to digital pin 3

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init RC522 
  pinMode(LED_PIN, OUTPUT); // Initialize LED pin as output
}

void loop() {
  // Check for new RFID card
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been read
  if (!rfid.PICC_ReadCardSerial())
    return;

  // Light up the LED
  digitalWrite(LED_PIN, HIGH);
  
  // Print UID to serial monitor
  Serial.print(F("RFID Tag UID:"));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  // Turn off the LED after 500 milliseconds
  delay(500);
  digitalWrite(LED_PIN, LOW);

  rfid.PICC_HaltA(); // Halt PICC
}

// Helper routine to print UID in HEX format
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}