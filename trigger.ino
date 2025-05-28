#include <SPI.h>
#include <RF24.h>
#include <Adafruit_NeoPixel.h>

#define CE_PIN 10
#define CSN_PIN 11
#define SCK_PIN 4
#define MISO_PIN 5
#define MOSI_PIN 6

// I/O
const int micPin = 1;
const int buttonPin = 7;
const int ledPin = 8; // RGB NeoPixel LED

// Threshold (gets set during calibration)
int soundThreshold = 500;

SPIClass spiBus(FSPI);
RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

Adafruit_NeoPixel pixel(1, ledPin, NEO_GRB + NEO_KHZ800);

void showColor(uint8_t r, uint8_t g, uint8_t b) {
  pixel.setPixelColor(0, pixel.Color(r, g, b));
  pixel.show();
}

void flashRedLED() {
  pixel.setPixelColor(0, pixel.Color(255, 0, 0)); 
  pixel.show();
  delay(100);
  pixel.clear();
  pixel.show();
}

void flashGreen(int duration = 250) {
  showColor(0, 255, 0); 
  delay(duration);
  pixel.clear();
  pixel.show();
}

int calibrateThreshold(int samples = 100, int margin = 10) { // adjust this margin for sensitivity relative to threshold
  long total = 0;

  showColor(0, 0, 255);  // Solid blue led during calibration

  for (int i = 0; i < samples; i++) {
    int val = analogRead(micPin);
    total += val;
    Serial.print("Ambient sample ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(val);
    delay(10);
  }

  pixel.clear();
  pixel.show();

  int average = total / samples;
  int threshold = average + margin;

  Serial.print("Calibrated ambient = ");
  Serial.print(average);
  Serial.print("-> Threshold = ");
  Serial.println(threshold);

  flashGreen(); 

  return threshold;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  analogReadResolution(12);
  pinMode(buttonPin, INPUT_PULLUP);

  pixel.begin();
  pixel.clear();
  pixel.show();

  spiBus.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CSN_PIN);
  if (!radio.begin(&spiBus)) {
    Serial.println("NRF24L01 not responding");
    while (1);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(100);
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("Calibrating sound threshold");
  soundThreshold = calibrateThreshold();

  Serial.println("ESP32 mic + manual trigger + auto-cal ready.");
}

void loop() {
  int micVal = analogRead(micPin);
  bool buttonPressed = digitalRead(buttonPin) == LOW;

  Serial.print("Mic: ");
  Serial.print(micVal);
  Serial.print(" | Button: ");
  Serial.println(buttonPressed ? "Pressed" : "Released");

  if (micVal > soundThreshold || buttonPressed) {
    const char msg[] = "TRIG";
    radio.write(&msg, sizeof(msg));
    Serial.println("Sent TRIG");

    flashRedLED();
    delay(100);
  } else {
    delay(50);
  }
}
