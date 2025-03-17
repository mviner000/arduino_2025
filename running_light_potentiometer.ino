#define NUM_LEDS 5
int ledPins[NUM_LEDS] = {2, 3, 4, 5, 6}; // Valid GPIOs for Arduino Uno
int potPin = A0; // Analog pin A0 for potentiometer

// Variables for non-blocking timing
unsigned long previousMillis = 0;
unsigned long interval = 100; // Default interval
int currentLed = 0;
bool goingForward = true;

void setup() {
  Serial.begin(9600); // Adjust baud rate for Uno
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
}

void loop() {
  // Read potentiometer and update interval immediately
  int potValue = analogRead(potPin); // Read potentiometer (0-1023)
  interval = map(potValue, 0, 1023, 20, 500); // Adjusted ADC range for Uno
  
  // Non-blocking timing control
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Turn off all LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
      digitalWrite(ledPins[i], LOW);
    }
    
    // Turn on current LED
    digitalWrite(ledPins[currentLed], HIGH);
    
    // Update the current LED position
    if (goingForward) {
      currentLed++;
      if (currentLed >= NUM_LEDS) {
        currentLed = NUM_LEDS - 2;
        goingForward = false;
      }
    } else {
      currentLed--;
      if (currentLed < 0) {
        currentLed = 1;
        goingForward = true;
      }
    }
  }
}
