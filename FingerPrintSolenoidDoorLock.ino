#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

#define Blue_LED   7   // Authorized
#define Red_LED    6   // Unauthorized
#define Solenoid   8   // Solenoid control pin

SoftwareSerial mySerial(2, 3); // RX, TX for fingerprint
Adafruit_Fingerprint finger(&mySerial);

unsigned long actionStartTime = 0;
bool actionInProgress = false;
bool authorized = false;

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for Serial on some boards
  delay(100);

  pinMode(Blue_LED, OUTPUT);
  pinMode(Red_LED, OUTPUT);
  pinMode(Solenoid, OUTPUT);
  

  digitalWrite(Blue_LED, LOW);
  digitalWrite(Red_LED, LOW);
  digitalWrite(Solenoid, HIGH); // Solenoid initially closed

  finger.begin(57600);
  delay(5);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains ");
  Serial.print(finger.templateCount);
  Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
}

void loop() {
  // If an action is in progress, check if 3 seconds passed
  if (actionInProgress && millis() - actionStartTime >= 3000) {
    // Reset everything
    digitalWrite(Solenoid, HIGH);  // Close solenoid
    digitalWrite(Blue_LED, LOW);
    digitalWrite(Red_LED, LOW);
    actionInProgress = false;
  }

  // If an action is in progress, skip scanning
  if (actionInProgress) return;

  uint8_t result = getFingerprintID();

  if (result == 99) { // Authorized
    Serial.println("Authorized fingerprint!");

    digitalWrite(Blue_LED, HIGH);
    digitalWrite(Solenoid, LOW); // Open solenoid

    actionStartTime = millis();
    actionInProgress = true;
    authorized = true;
  } else if (result == 0xFF) {
    // No finger or failed read, do nothing
  } else {
    Serial.println("Unauthorized fingerprint!");

    digitalWrite(Red_LED, HIGH);
    digitalWrite(Solenoid, HIGH); // Make sure solenoid is closed

    actionStartTime = millis();
    actionInProgress = true;
    authorized = false;
  }
}

// Fingerprint reader logic
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return 0xFF;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return 0xFF;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return 0xFE;

  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence "); Serial.println(finger.confidence);
  return finger.fingerID;
}
