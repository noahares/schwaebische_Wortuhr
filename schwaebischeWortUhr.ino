#include <EEPROM.h>
#include <Timer.h>
#include <Wire.h>

int eckMin = 0; //Minutenvariable für Eck-LEDs
int clearVal = 0; //Variable zum aktualisieren der Uhr

char btVal; //Empfangener Bluetooth-Code
int brightCount = 0; //Helligkeitsstufen
int brightness = 0; //Helligkeit
int color = 0; //Variable für Farbe

//eeprom adressen
int colAddr = 0;
int brightAddr = 0;

/*
all hours
*/
int[] oise = {62,63,64,65}
int[] zwoie = {66,67,68,69,70};
int[] dreie = {83,84,85,86,87};
int[] viere = {100,101,102,103,99};
int[] fuenfe = {49,50,51,52,53};
int[] sechse = {58,59,60,61,62,63};
int[] siebne = {104,105,106,107,108,109};
int[] achte = {71,72,73,74,75};
int[] neune = {91,92,93,94,95};
int[] zehne = {88,89,90,91,92};
int[] elfe = {55,56,57,58};
int[] zwoelfe = {77,78,79,80,81,82};

/*
all parts of an hour
*/
int[] vor = {33,34,35};
int[] nach = {40,41,42,43};
int[] fuenf = {29,30,31,32}
int[] zehn = {22,23,24,25};
int[] viertl = {12,13,14,15,16,17};
int[] halb = {44,45,46,47};
int[] dreiviertl = {12,13,14,15,16,17,18,19,20,21};

//es isch
int[] start = {0,1,2,3,4,5,6};

int[] A = {108,89,85,68,62,47,39,26,16,28,37,51,58,74,73,72,71,70,69,79,97,100};
int[] E = {107,106,105,104,103,102,101,90,85,68,63,46,41,24,19,18,17,16,15,14,13,62,61,60,59,58,57};
int[] F = {107,90,85,68,63,46,41,24,19,18,17,16,15,14,13,62,61,60,59};
int[] H = {107,90,85,68,63,46,41,24,19,13,30,35,52,57,74,79,96,101,57,58,59,60,61,62,63};
int[] I = {104,93,82,71,90,49,38,27,16};
int[] L = {90,85,86,63,46,41,24,19,107,106,105,104,103,102,101};
int[] M = {108,89,86,67,64,45,42,23,20,24,40,48,60,50,36,30,12,31,34,53,56,75,78,97,100};
int[] N = {108.89,86,67,64,45,42,23,20,24,40,48,60,72,80,96,100,97,78,75,56,53,34,31,12};
int[] O = {17,25,41,45,64,67,85,91,105,104,103,95,79,75,56,53,35,29,15,16};
int[] R = {107,90,85,68,63,46,41,24,19,18,17,16,15,14,30,34,52,58,59,60,61,62,72,80,96,100};
int[] S = {108,107,106,105,104,103,102,96,78,74,58,59,60,61,62,46,42,24,18,17,16,15,14,13,12};
int[] T = {104,93,82,71,90,49,38,27,16,15,14,13,12,17,18,19,20};

int[] NO = {36,37};
int[] TS = {38,39};
int[] FLO = {26,27,28};
int[] FK = {96,97};
int[] EULE = {7,8,9,10};

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN            9 //Pin des LED-Bands
#define NUMPIXELS      114 //Anzahl der LEDs
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //Inizialisierung der NeoPixels Bibliothek
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN , NEO_GRB + NEO_KHZ800);

#define DS3231_I2C_ADDRESS 0x68 //Adresse zur Kommunikation mit RTC

Timer t;
int tickEvent; //Tmer zur Aktualisierung
int hours;  //Stundenvariable
int minutes; //Minutenvariable


//Start-Uhrzeit des RTCs inizialisieren
void setDS3231time(byte second, byte minute, byte hour) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);
  Wire.write(decToBcd(second)); //sekunden einstellen
  Wire.write(decToBcd(minute)); //minuten einstellen
  Wire.write(decToBcd(hour)); //stunden einstellen
  Wire.endTransmission();
}

//konvertiert Dezimalcode in binären Code
byte decToBcd(byte val) {
  return ( (val / 10 * 16) + (val % 10) );
}

//konvertiert binären Code in Dezimalcode
byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}

void EEPROMWriteCol(int colAddr, int color) { //store a 2 byte integer at the eeprom at the specified address and address + 1
  byte lowByte = ((color >> 0) & 0xFF);
  byte highByte = ((color >> 8) & 0xFF);

  EEPROM.update(colAddr, lowByte);
  EEPROM.update(colAddr + 1, highByte);
}

unsigned int EEPROMReadCol(int colAddr) { //read a 2 byte integer from the eeprom at the specified address and address + 1
  byte lowByte = EEPROM.read(colAddr);
  byte highByte = EEPROM.read(colAddr + 1);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

void EEPROMWriteBright(int brightAddr, int brightCount) { //store a 2 byte integer at the eeprom at the specified address + 2 and address + 3
  byte lowByte = ((brightCount >> 0) & 0xFF);
  byte highByte = ((brightCount >> 8) & 0xFF);

  EEPROM.update(brightAddr + 2, lowByte);
  EEPROM.update(brightAddr + 3, highByte);
}

unsigned int EEPROMReadBright(int brightAddr) { // read a 2 byte integer from the eeprom at the specified address + 2 and address + 3
  byte lowByte = EEPROM.read(brightAddr + 2);
  byte highByte = EEPROM.read(brightAddr + 3);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}
void setup() {
  pixels.begin(); //LEDs werden eingebunden
  strip.begin();
  Wire.begin(); //kommunikation wird eingerichtet
  tickEvent = t.every(1000, displayTime); //alle 1000ms wird displayTime ausgeführt
  Serial.begin(9600); //Serieller Monitor wird gestartet
  Serial1.begin(9600); //Serielle Kommunikation mit Bluetooth-Modul wird gestartet
  randomSeed(A1); //Zufallsvariabe
  // aktuelle Zeit beim ersten Hochladen eingeben, danach auskommentieren und erneut hochladen
  // DS3231 sekunden, minuten, stunden
  // setDS3231time(39,03,12);
}

void readDS3231time(byte *second, byte *minute, byte *hour) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // gibt O Uhr vor
  Wire.endTransmission();

  //3 Bytes werden vom RTC abgefragt
  Wire.requestFrom(DS3231_I2C_ADDRESS, 3);
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
}

void displayTime() {
  byte second, minute, hour;
  readDS3231time(&second, &minute, &hour); //RTC wird ausgelesen

  btCkeck();

  //Minuten und Stunden werden in Decimalcode umgewandet und an den seriellen Monitor geschickt
  minutes = (minute) - 0;
  Serial.print(hour, DEC);
  Serial.print(":");
  if (minutes < 10) Serial.print("0");
  Serial.println(minutes);
  if (minutes < 15) hours = (hour) - 0;
  else hours = ((hour) - 0) + 1;

  eckMin = minutes % 5; //Eckminuten werden aus Minuten geteilt durch 5 berechnet; der ganzzahlige Rest liegt somit zwischen 0 und 4
  if (eckMin == 0 && clearVal == 0) {
    pixels.clear(); //immer wenn eine neue 5min Phase beginnt, wird die Anzeige der Uhr aktualisiert
    clearVal = 1; //clearVal wird auf 1 gesetzt, damit das Display bei vollen 5min nicht schnell blinkt, da es immer wider aktualisiert wird
  }
  if (eckMin == 1) {
    displayContent({111});
    clearVal = 0; //clearVal wird für den nähsten Zyklus wieder auf 0 gesetzt
  }
  if (eckMin == 2) {
    displayContent({111,112});
  }
  if (eckMin == 3) {
    displayContent({111,112,113});
  }
  if (eckMin == 4) {
    displayContent({110,111,112,113});
  }

  displayContent(start);

  //Zeitabschnitte zum Anzeigen von viertl, halb, dreiviertl, zehn, fünf, vor und nach werden definiert
  if (minutes >= 15 && minutes < 20) displayContent(viertl);
  if (minutes >= 10 && minutes < 15) {
    displayContent(zehn);
    displayContent(nach);
  }
  if (minutes >= 5 && minutes < 10) {
    displayContent(fuenf);
    displayContent(nach);
  }
  if (minutes >= 30 && minutes < 35) displayContent(halb);
  if (minutes >= 20 && minutes < 25) {
    displayContent(zehn);
    displayContent(vor);
    displayContent(halb);
  }
  if (minutes >= 25 && minutes < 30) {
    displayContent(fuenf);
    displayContent(vor);
    displayContent(halb);
  }
  if (minutes >= 40 && minutes < 45) {
    displayContent(zehn);
    displayContent(nach);
    displayContent(halb);
  }
  if (minutes >= 35 && minutes < 40) {
    displayContent(fuenf);
    displayContent(nach);
    displayContent(halb);
  }
  if (minutes >= 45 && minutes < 50) displayContent(dreiviertl);
  if (minutes >= 50 && minutes < 55) {
    displayContent(zehn);
    displayContent(vor);
  }
  if (minutes >= 55 && minutes < 60) {
    displayContent(fuenf);
    displayContent(vor);
  }

  //Stunden-Anzeige wird zugewiesen
  if (hours > 12) hours = hours - 12;
  if (hours == 0) displayContent(zwoelfe);
  if (hours == 1) displayContent(oise);
  if (hours == 2) displayContent(zwoie);
  if (hours == 3) displayContent(dreie);
  if (hours == 4) displayContent(viere);
  if (hours == 5) displayContent(fuenfe);
  if (hours == 6) displayContent(sechse);
  if (hours == 7) displayContent(siebne);
  if (hours == 8) displayContent(achte);
  if (hours == 9) displayContent(neune);
  if (hours == 10) displayContent(zehne);
  if (hours == 11) displayContent(elfe);
  if (hours == 12) displayContent(zwoelfe);

  //optional zur Ausgabe von Sekunden auf dem seriellen Monitor
  /*Serial.print(":");
    if (second<10)
    {
    Serial.print("0");
    }
    Serial.println(second, DEC);*/

}
void loop() {
  t.update(); //Update-Funktion wird bei jeden loop ausgeführt
}


void displayContent(int[] leds)  {
  for (int l : leds) {
    leuchten(l);
  }
  pixels.show();
}

void btCheck() {
  color = EEPROMReadCol(colAddr);
  brightCount = EEPROMReadBright(brightAddr);
  if (Serial1.available()) //wenn Bluetooth-Signal empfangen...
  {
    pixels.clear(); //...wird die Anzeige aktualisiert...
    btVal = Serial1.read(); //... und der empfangene Bluetooth-Code ausgelesen
  }

  //je nach emfpangenen Signal wird die Farbe geändert
  if (btVal == '1') color = 1;
  if (btVal == '2') color = 2;
  if (btVal == '3') color = 3;
  if (btVal == '4') color = 4;
  if (btVal == '5') color = 5;
  if (btVal == '6') color = 6;
  if (btVal == '7') color = 7;
  if (btVal == '8') color = 8;
  EEPROMWriteCol(colAddr, color);
  if (btVal == '0') {
    brightCount++; //zähler für Heligkeitsstufen wird erhöht
    btVal = '9'; //beliebige unbenutzte Zahl, die verhindert, dass mehrere Helligkeitsstufen au einmal durchlaufen werden
    EEPROMWriteBright(brightAddr, brightCount);
  }

  //Heligkeit wird je nach Rest auf Helligkeitszähler und 6 verändert
  switch (brightCount % 6) {
    case 0: brightness = 60;
      break;
    case 1: brightness = 45;
      break;
    case 2: brightness = 30;
      break;
    case 3: brightness = 15;
      break;
    case 4: brightness = 90;
      break;
    case 5: brightness = 75;
      break;
    default: brightness = 60; //standard
      break;
    }


}

void draw(int[] leds) {
  pixels.clear();
  for(int i : leds) {
    pixel.setPixelColor(i, pixelColor(brightness,brightness,brightness));
  }
  pixel.show();
}

void leuchten(int n) {

  //ausgehend von der Farbe werden alle momentan aktiven LEDs auf diese Farbe umprogrammiert
  switch (color) {
    case 1:
      pixels.setPixelColor(n, pixels.Color(0, 0, brightness)); //blau
      break;
    case 2:
      pixels.setPixelColor(n, pixels.Color(brightness, 0, 0)); //rot
      break;
    case 3:
      pixels.setPixelColor(n, pixels.Color(0, brightness, 0)); //grün
      break;
    case 4:
      pixels.setPixelColor(n, pixels.Color(brightness, brightness, 0)); //gelb
      break;
    case 5:
      pixels.setPixelColor(n, pixels.Color(brightness, 0, brightness)); //lila
      break;
    case 6:
      pixels.setPixelColor(n, pixels.Color(0, brightness, brightness)); //hellblau
      break;
    case 7:
      pixels.setPixelColor(n, pixels.Color(brightness, brightness, brightness)); //weiss
      break;
    case 8:
      pixels.setPixelColor(n, pixels.Color(random(0, brightness), random(0, brightness), random(0, brightness))); //strobo
      break;
    default:
      pixels.setPixelColor(n, pixels.Color(0, 0, brightness)); //wenn kein Signal passt, dann blau
      break;
  }



  if (btVal == 'a') draw(FLO);


  if (btVal == 'n') draw(NO);

  if (btVal == 't') draw(TS);

  if (btVal == 'N') {
    draw(N);
    delay(1000);
    draw(O);
    delay(1000);
    draw(A);
    delay(1000);
    draw(H);
    delay(1000);
    pixels.clear();
  }

  if (btVal == 'E') { //elena
    draw(E);
    delay(1000);
    draw(L);
    delay(1000);
    draw(E);
    delay(1000);
    draw(N);
    delay(1000);
    draw(A);
    delay(1000);
    TODO HERZ impl
    //draw(HERZ);
    //delay(1000);
    pixels.clear();
  }

  if (btVal == 'f') draw(FK);

  if (btVal == 'h') { //herz
    pixels.clear();
    pixels.setPixelColor(104, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(94, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(92, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(80, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(84, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(74, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(68, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(56, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(64, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(54, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(44, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(43, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(33, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(31, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(23, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(38, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(28, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(26, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(14, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(13, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(18, pixels.Color(brightness, 0, 0));
    pixels.setPixelColor(19, pixels.Color(brightness, 0, 0));
  }

  if (btVal == 'x') { //crazy shit
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(random(0, brightness), random(0, brightness), random(0, brightness)));
      delay(5);
    }
    pixels.show();
  }

  if (btVal == 'y') { //other crazy shit
    colorWipe(strip.Color(brightness, 0, 0), 20); // Red
    colorWipe(strip.Color(0, brightness, 0), 20); // Green
    colorWipe(strip.Color(0, 0, brightness), 20); // Blue
  }

  /*if (btVal == 'z'){
    rainbow(10);

    }
  */
}

void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    if (Serial1.available()) //wenn Bluetooth-Signal empfangen...
    {
      goto start;
    }
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
start:
  pixels.clear();
}

/*void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
      if (Serial1.available()) //wenn Bluetooth-Signal empfangen...
  {
    goto start1;
  }
    }
    strip.show();
    delay(wait);
    start1:
    pixels.clear();
  }

  }




  // Input a value 0 to 255 to get a color value.
  // The colours are a transition r - g - b - back to r.
  uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos*3, 0, WheelPos*3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos*3,255 - WheelPos*3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos*3, 255 - WheelPos*3, 0);
  }*/
