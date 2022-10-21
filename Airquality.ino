#define BLYNK_AUTH_TOKEN "Pdy_rqYFYK6PGIqvqC4_rTo9EZujFDFF"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include "Adafruit_SGP30.h"
Adafruit_SGP30 sgp;
#define BLYNK_PRINT Serial
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);    

int dustPin = A0; 
int ledPin = D3;  
int buzzer = D6; 
float voltsMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;
int counter = 0;
int co2,tvoc,rawH2,rawEth;

char auth[] = BLYNK_AUTH_TOKEN;
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "vivo1902";
char pass[] = "kishan27";

BlynkTimer timer;

void sendValues()
{
  Blynk.virtualWrite(V4, dustDensity);
  Blynk.virtualWrite(V5, co2);
  Blynk.virtualWrite(V6, tvoc);
  Blynk.virtualWrite(V7, rawH2);
  Blynk.virtualWrite(V8, rawEth);
}

void setup()
{
  Serial.begin(9600);
  pinMode(ledPin,OUTPUT);
  pinMode(dustPin,INPUT);
  pinMode(buzzer,OUTPUT);
  Wire.begin();  
  sgp.begin();
  if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }
  Serial.print("Found SGP30 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
  
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  digitalWrite(buzzer,LOW);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(3000L, sendValues);
}
void loop(){ 
  Blynk.run();
  timer.run();
  dustSensor();
  delay(1000);
  airQuality();
  delay(1000);
}
void dustSensor(){
  digitalWrite(ledPin,LOW);
  delayMicroseconds(280);
  voltsMeasured = analogRead(dustPin);
  delayMicroseconds(40);
  digitalWrite(ledPin,HIGH);
  delayMicroseconds(9680);
 
  calcVoltage = voltsMeasured * (3.3 / 1024.0);
  dustDensity = 1000 * ((0.34 * calcVoltage)-0.05); // subtaract -0.1 inside bracket if required to caliberate
  Serial.println("GP2Y1010AU0F readings"); 
  Serial.print("Dust Density = ");
  Serial.println(dustDensity);
  Serial.println();
  lcd.clear();
  lcd.setCursor(0,0);   
  lcd.print("Dustdensity:"); lcd.print(dustDensity);
  if(dustDensity > 200){
  digitalWrite(buzzer,HIGH);
  Blynk.notify("Dust density is high");
  }
  else
  digitalWrite(buzzer,LOW);
  delay(10);
}
void airQuality(){
  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  co2 = sgp.eCO2; tvoc = sgp.TVOC; rawH2 = sgp.rawH2; rawEth = sgp.rawEthanol;
  Serial.print("TVOC "); Serial.print(tvoc); Serial.print(" ppb\t");
  Serial.print("eCO2 "); Serial.print(co2); Serial.println(" ppm");
  
  if (! sgp.IAQmeasureRaw()) {
    Serial.println("Raw Measurement failed");
    return;
  }
  Serial.print("Raw H2 "); Serial.print(rawH2); Serial.print(" \t");
  Serial.print("Raw Ethanol "); Serial.print(rawEth); Serial.println("");

  lcd.clear();
  lcd.setCursor(0,1);   //Move cursor to character 2 on line 1
  lcd.print("eCO2 "); lcd.print(co2); lcd.print(" ppm");
  lcd.setCursor(0,0);
  lcd.print("TVOC "); lcd.print(tvoc); lcd.print(" ppb");
  if(co2 > 1000){ 
  digitalWrite(buzzer,HIGH);
  Blynk.notify("Co2 is high");
  }
  else
  digitalWrite(buzzer,LOW);
int counter = 0;
  delay(50);
  counter++;
  if (counter == 3000) {
    counter = 0;
    uint16_t TVOC_base, eCO2_base;
    if (! sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
      Serial.println("Failed to get baseline readings");
      return;
    }
    Serial.print("****Baseline values: eCO2: 0x"); Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
  }
}
