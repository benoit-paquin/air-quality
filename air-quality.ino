// Includes
#include <Wire.h> //I2C interface
#include <SoftwareSerial.h>
//Declarations
SoftwareSerial pmsSerial(2, 3);
int ledground = 8;
int green = 4;
int yellow = 5;
int red = 6;
int blue = 7;

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

void blinkColor(int color, int cnt) {
  for (int i = 0; i < cnt; i++) {
    digitalWrite(color, HIGH);
    delay(500);
    digitalWrite(color, LOW);
    delay(500);
  }
}
void blink(int cnt) {
  if (cnt >= 100) {
    blinkColor(blue, cnt / 100);
  cnt = cnt - 100*(cnt / 100);
  }  
  if (cnt >= 25) {
    blinkColor(red, cnt / 25);
  }
  cnt = cnt - 25*(cnt / 25);
  if (cnt >= 5) {
    blinkColor(yellow, cnt / 5);
  }
  cnt = cnt - 5*(cnt / 5);
  if (cnt >= 1) {
    blinkColor(green, cnt);
  }
}
void blinkStart() {
  //blink the 3 leds in a row to indicate a new reading
  delay(400);
  digitalWrite(blue,HIGH);
  delay(70);
  digitalWrite(blue,LOW);
  digitalWrite(red,HIGH);
  delay(70);
  digitalWrite(red,LOW);
  digitalWrite(yellow,HIGH);
  delay(70);
  digitalWrite(yellow,LOW);
  digitalWrite(green,HIGH);
  delay(70);
  digitalWrite(green,LOW);  
  delay(400);
}
void setup() {
  pinMode(ledground, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(blue, OUTPUT);
  digitalWrite(ledground, LOW);
  digitalWrite(green, LOW);
  digitalWrite(yellow, LOW);
  digitalWrite(red, LOW);
  digitalWrite(blue, LOW);
  // our debugging output
  Serial.begin(9600);
  // sensor baud rate is 9600
  pmsSerial.begin(9600);
  Wire.begin();
}

struct pms5003data data;

void loop() {
  if (readPMSdata(&pmsSerial)) {
    // reading data was successful!
    delay(1000);
    blinkStart();
    blink(data.pm100_standard);
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (standard)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_standard);
    Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_standard);
    Serial.print("\t\tPM 10: "); Serial.println(data.pm100_standard);
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (environmental)");
    Serial.print("PM 1.0: "); Serial.print(data.pm10_env);
    Serial.print("\t\tPM 2.5: "); Serial.print(data.pm25_env);
    Serial.print("\t\tPM 10: "); Serial.println(data.pm100_env);
    Serial.println("---------------------------------------");
    Serial.print("Particles > 0.3um / 0.1L air:"); Serial.println(data.particles_03um);
    Serial.print("Particles > 0.5um / 0.1L air:"); Serial.println(data.particles_05um);
    Serial.print("Particles > 1.0um / 0.1L air:"); Serial.println(data.particles_10um);
    Serial.print("Particles > 2.5um / 0.1L air:"); Serial.println(data.particles_25um);
    Serial.print("Particles > 5.0um / 0.1L air:"); Serial.println(data.particles_50um);
    Serial.print("Particles > 10.0 um / 0.1L air:"); Serial.println(data.particles_100um);
    Serial.println("---------------------------------------");
  }
}

boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }

  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }

  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }

  uint8_t buffer[32];
  uint16_t sum = 0;
  s->readBytes(buffer, 32);

  // get checksum ready
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }

  /* debugging
    for (uint8_t i=2; i<32; i++) {
    Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
    }
    Serial.println();
  */

  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  // put it into a nice struct :)
  memcpy((void *)&data, (void *)buffer_u16, 30);

  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }
  // success!
  return true;
}
