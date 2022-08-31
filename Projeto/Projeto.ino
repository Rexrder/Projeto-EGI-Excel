//LDR
const int ldrPin = A0;

//DHT11
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT11
#include <SPI.h>
#include <SD.h>
DHT dht(DHTPIN, DHTTYPE);

File myFile;

//MPU-6050
#include<Wire.h>
const int MPU_addr = 0x68; // I2C address of the MPU-6050
int AcX = 0;
int AcY = 0;
int AcZ = 0;
int Tmp = 0;
int GyX = 0;
int GyY = 0;
int GyZ = 0;
bool save = false;
bool upload = false;

int iter = 0; //var to define in which iteration the info is uploaded

//Led verde
#define GREEN 6 //pin Led
#define LOAD 7 //pin load

void setup() {
  attachInterrupt(digitalPinToInterrupt(3), mod, RISING); //interrupt to start/stop reading
  pinMode(LOAD, INPUT);

  //LDR
  pinMode(ldrPin, INPUT);

  //LED GREEN
  pinMode(GREEN, OUTPUT);
  digitalWrite(GREEN, LOW);

  Serial.begin(9600);

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }

  if (digitalRead(LOAD) == HIGH) { //if uploade mode active
    myFile = SD.open("data.txt", FILE_READ);
    upload = true;
  }
  else {

    myFile = SD.open("data.txt", FILE_WRITE);
  }

  //DHT11
  dht.begin();


  //MPU-6050
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}

void loop() {

  change();

  //save_data mode

  if (digitalRead(LOAD) == LOW) {
    upload = false;
    //Led verde
    if (save) {
      analogWrite(GREEN, HIGH);
    } else {
      analogWrite(GREEN, LOW);
    }

    //MPU-6050
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
    int t_AcX, t_AcY, t_AcZ, t_GyX, t_GyY, t_GyZ;

    //read accel data
    t_AcX = (Wire.read() << 8 | Wire.read());
    t_AcY = (Wire.read() << 8 | Wire.read());
    t_AcZ = (Wire.read() << 8 | Wire.read());

    //read temperature data
    Tmp = (Wire.read() << 8 | Wire.read());

    //read gyro data
    t_GyX = (Wire.read() << 8 | Wire.read());
    t_GyY = (Wire.read() << 8 | Wire.read());
    t_GyZ = (Wire.read() << 8 | Wire.read());

    //save most significant
    bigger(AcX, t_AcX);
    bigger(AcY, t_AcY);
    bigger(AcZ, t_AcZ);
    bigger(GyX, t_GyX);
    bigger(GyY, t_GyY);
    bigger(GyZ, t_GyZ);

    iter++;

    if (iter >= 4) {
      iter = 0;

      //LDR
      int is_open = 0;
      int ldrStatus = analogRead(ldrPin);
      if (ldrStatus > 250) {
        is_open = 1;
      }

      //DHT11
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      /*if (isnan(h) || isnan(t) ) {
        Serial.println(F("ERRO"));
        return;
        }
        Serial.print(F("Humidade: "));
        Serial.print(h);
        Serial.print(F("%  Temperatura: "));
        Serial.print(t);
        Serial.println(F("°C "));

        Serial.print("Accelerometer: ");
        Serial.print("X = "); Serial.print(AcX);
        Serial.print(" | Y = "); Serial.print(AcY);
        Serial.print(" | Z = "); Serial.println(AcZ);

        Serial.print("Gyroscope: ");
        Serial.print("X = "); Serial.print(GyX);
        Serial.print(" | Y = "); Serial.print(GyY);
        Serial.print(" | Z = "); Serial.println(GyZ);

        Serial.print("Open: ");
        Serial.print("True = "); Serial.print(is_open);

        Serial.println(" ");*/

      double data[] = {t, h, AcX, AcY, AcZ, GyX, GyY, GyZ};

      //send array of info into SD card

      if (save && myFile) {
        for (int i = 0; i < (sizeof(data) / sizeof(data[0])); i++) {
          myFile.print(data[i]);
          myFile.print(" ");
        }
        myFile.print(is_open);
        myFile.println();
        myFile.flush();
      }

      //restart var values

      AcX = 0;
      AcY = 0;
      AcZ = 0;
      GyX = 0;
      GyY = 0;
      GyZ = 0;

    }

    delay(500);

  } else {

    //upload mode

    while (!Serial.available() && digitalRead(LOAD) == HIGH) {
      delay(10);
    }
    if (digitalRead(LOAD) == HIGH) {
      upload = true;
      int inByte = Serial.read();
      inByte = Serial.read();

      if (SD.exists("data.txt")) {
        while (myFile.available()) {
          Serial.write(myFile.read());
        }
        Serial.println();
        Serial.println("e");
        myFile.close();
        SD.remove("data.txt");
      }
      else {
        Serial.println();
        Serial.println("e");
      }
    }
  }
}

void bigger(int&a, int b) { // Choose the most extreme values for the output
  a = (abs(a) > abs(b)) ? a : b;
}

void mod() { //read or not info
  save = !save;
}


void change() { //when present state differs from previous, close file and open in new mode (write/read)
  if ( upload && digitalRead(LOAD) == LOW ) {
    myFile.close();
    myFile = SD.open("data.txt", FILE_WRITE);
  }
  if ( !upload && digitalRead(LOAD) == HIGH ) {
    analogWrite(GREEN, LOW);
    myFile.close();
    myFile = SD.open("data.txt", FILE_READ);
  }
}
