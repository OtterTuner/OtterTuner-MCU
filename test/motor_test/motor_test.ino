void setup() {
	Serial.begin(115200);

  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  Serial.println("setup complete");
}

void loop() {
  digitalWrite(6, HIGH);
  digitalWrite(7, LOW);
  
  Serial.println("1");
  delay(2000);

  digitalWrite(6, LOW);
  digitalWrite(7, LOW);

  Serial.println("2");
  delay(2000);

  digitalWrite(6, LOW);
  digitalWrite(7, HIGH);
  
  Serial.println("3");
  delay(2000);

  digitalWrite(6, LOW);
  digitalWrite(7, LOW);

  Serial.println("4");
  delay(2000);
}
