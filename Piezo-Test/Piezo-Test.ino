const int PIEZO_PIN = 2;

void setup() {
  // put your setup code here, to run once:
  pinMode(PIEZO_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  tone(PIEZO_PIN, 1000);
  delay(1000);
  noTone(PIEZO_PIN);
  delay(1000);
}
