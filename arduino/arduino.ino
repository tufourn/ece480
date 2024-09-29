void setup() {
  Serial.begin(115200);
  while (Serial.available() <= 0) {
    Serial.print('X');
    delay(100);
  }
}

void loop() {
  if (Serial.available() > 0) {
    int incomingByte = Serial.read();
    Serial.print('R'); // let host know byte has been received
  }
}
