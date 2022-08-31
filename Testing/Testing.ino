
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while(!Serial.available()){
    delay(10);
  }
  
  for (int i = 0; i < 30; i++){
    for (int n = 0; n < 8; n++){
      Serial.print(rand()%20);
      Serial.print(" ");
    }
    Serial.println();
  }

  Serial.println("e");
  
}

void loop() {
  // nothing happens after setup
}
