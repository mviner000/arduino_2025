#include <TM1637Display.h>

const int CLK_PIN = 2;
const int DIO_PIN = 3;
const int BUZZER_PIN = 4; // Define buzzer pin
const int BRIGHTNESS_LEVEL = 2; // Value between 0-7

TM1637Display display(CLK_PIN, DIO_PIN);
unsigned long totalSeconds = 60; // Start countdown from 1 minute

void setup() {
  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer as output
  digitalWrite(BUZZER_PIN, LOW); // Ensure it's off at the start
  display.setBrightness(BRIGHTNESS_LEVEL);
  displayTime(totalSeconds);
}

void loop() {
  if (totalSeconds > 0) {
    displayTime(totalSeconds);

    // If countdown is 10 seconds or less, beep the buzzer
    if (totalSeconds <= 10) {
      digitalWrite(BUZZER_PIN, HIGH); // Turn on buzzer
      delay(200);                     // Beep duration
      digitalWrite(BUZZER_PIN, LOW);  // Turn off buzzer
    }

    totalSeconds--;
    delay(1000); // Update every second
  } else {
    // Show "00:00" and stop countdown
    display.showNumberDecEx(0, 0b01000000, true);

    // Optional: Long beep when countdown reaches 0
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);

    while (true); // Stop execution
  }
}

void displayTime(unsigned long totalSeconds) {
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;

  uint8_t data[] = {
    display.encodeDigit(minutes / 10),       // Minutes tens place
    display.encodeDigit(minutes % 10) | 0x80, // Minutes units place with colon
    display.encodeDigit(seconds / 10),       // Seconds tens place
    display.encodeDigit(seconds % 10)        // Seconds units place
  };

  display.setSegments(data);
}
