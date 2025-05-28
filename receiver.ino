#include <SPI.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10
#define TRIGGER_PIN 3  // optocoupler

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  pinMode(TRIGGER_PIN, OUTPUT);
  digitalWrite(TRIGGER_PIN, LOW);

  if (!radio.begin()) {
    Serial.println("NRF24L01 not responding!");
    while (1);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(100);
  radio.openReadingPipe(1, address);
  radio.startListening();

  Serial.println("Nano receiver + optocoupler ready.");
}

void loop() {
  if (radio.available()) {
    char msg[10] = "";
    radio.read(&msg, sizeof(msg));
    Serial.print("Received: ");
    Serial.println(msg);

    if (strcmp(msg, "TRIG") == 0) {
      digitalWrite(TRIGGER_PIN, HIGH);  // triggers optocoupler
      delay(1000);                       
      digitalWrite(TRIGGER_PIN, LOW);
    }
  }
}
