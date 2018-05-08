#include <EEPROM.h>
#include <Timer.h>
#include <Wire.h>

int eckMin = 0; //Minutenvariable für Eck-LEDs
int clearVal = 0; //Variable zum aktualisieren der Uhr

char btVal; //Empfangener Bluetooth-Code
int brightCount = 0; //Helligkeitsstufen
int brightness = 0; //Helligkeit

int color; //Variable für Farbe

int colAddr = 0;
int brightAddr = 0;

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
void setDS3231time(byte second,
                   byte minute,
                   byte hour)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);
  Wire.write(decToBcd(second)); //sekunden einstellen
  Wire.write(decToBcd(minute)); //minuten einstellen
  Wire.write(decToBcd(hour)); //stunden einstellen
  Wire.endTransmission();
}

//konvertiert Dezimalcode in binären Code
byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}

//konvertiert binären Code in Dezimalcode
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

void EEPROMWriteCol(int colAddr, int color) //store a 2 byte integer at the eeprom at the specified address and address + 1

{
  byte lowByte = ((color >> 0) & 0xFF);
  byte highByte = ((color >> 8) & 0xFF);

  EEPROM.update(colAddr, lowByte);
  EEPROM.update(colAddr + 1, highByte);
}

unsigned int EEPROMReadCol(int colAddr) //read a 2 byte integer from the eeprom at the specified address and address + 1

{
  byte lowByte = EEPROM.read(colAddr);
  byte highByte = EEPROM.read(colAddr + 1);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

void EEPROMWriteBright(int brightAddr, int brightCount) //store a 2 byte integer at the eeprom at the specified address + 2 and address + 3
{
  byte lowByte = ((brightCount >> 0) & 0xFF);
  byte highByte = ((brightCount >> 8) & 0xFF);

  EEPROM.update(brightAddr + 2, lowByte);
  EEPROM.update(brightAddr + 3, highByte);
}

unsigned int EEPROMReadBright(int brightAddr) // read a 2 byte integer from the eeprom at the specified address + 2 and address + 3

{
  byte lowByte = EEPROM.read(brightAddr + 2);
  byte highByte = EEPROM.read(brightAddr + 3);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}
void setup()
{
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

void readDS3231time(byte *second,
                    byte *minute,
                    byte *hour)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // gibt O Uhr vor
  Wire.endTransmission();

  //3 Bytes werden vom RTC abgefragt
  Wire.requestFrom(DS3231_I2C_ADDRESS, 3);
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
}
void displayTime()
{
  byte second, minute, hour;
  readDS3231time(&second, &minute, &hour); //RTC wird ausgelesen

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
    leuchten(111); //die Funktion leuchten wird mit dem Parameter n aufgerufen; n steht fr die LED di leuchten soll
    clearVal = 0; //clearVal wird für den nähsten Zyklus wieder auf 0 gesetzt
  }
  if (eckMin == 2) {
    leuchten(111);
    leuchten(112);
  }
  if (eckMin == 3) {
    leuchten(111);
    leuchten(113);
    leuchten(112);
  }
  if (eckMin == 4) {
    leuchten(110);
    leuchten(111);
    leuchten(112);
    leuchten(113);
  }

  leuchten(0);
  leuchten(1);
  leuchten(3);
  leuchten(4);
  leuchten(5);
  leuchten(6);

  //Zeitabschnitte zum Anzeigen von viertl, halb, dreiviertl, zehn, fünf, vor und nach werden definiert
  if (minutes >= 15 && minutes < 20) viertl();

  if (minutes >= 10 && minutes < 15) {
    zehn();
    nach();
  }
  if (minutes >= 5 && minutes < 10) {
    fuenf();
    nach();
  }

  if (minutes >= 30 && minutes < 35) halb();

  if (minutes >= 20 && minutes < 25) {
    zehn();
    vor();
    halb();
  }

  if (minutes >= 25 && minutes < 30) {
    fuenf();
    vor();
    halb();
  }

  if (minutes >= 40 && minutes < 45) {
    zehn();
    nach();
    halb();
  }

  if (minutes >= 35 && minutes < 40) {
    fuenf();
    nach();
    halb();
  }

  if (minutes >= 45 && minutes < 50) dreiviertl();

  if (minutes >= 50 && minutes < 55) {
    zehn();
    vor();
  }

  if (minutes >= 55 && minutes < 60) {
    fuenf();
    vor();
  }

  //Stunden-Anzeige wird zugewiesen
  if (hours > 12) hours = hours - 12;
  if (hours == 0) zwoelfe();
  if (hours == 1) oise();
  if (hours == 2) zwoie();
  if (hours == 3) dreie();
  if (hours == 4) viere();
  if (hours == 5) fuenfe();
  if (hours == 6) sechse();
  if (hours == 7) siebne();
  if (hours == 8) achte();
  if (hours == 9) neune();
  if (hours == 10) zehne();
  if (hours == 11) elfe();
  if (hours == 12) zwoelfe();

  //optional zur Ausgabe von Sekunden auf dem seriellen Monitor
  /*Serial.print(":");
    if (second<10)
    {
    Serial.print("0");
    }
    Serial.println(second, DEC);*/

}
void loop()
{
  t.update(); //Update-Funktion wird bei jeden loop ausgeführt
}

//zuweisen der LEDs, die für die einzelnen Wörter leuchten sollen
void vor() {
  leuchten(33);
  leuchten(34);
  leuchten(35);
}

void nach() {
  leuchten(40);
  leuchten(41);
  leuchten(42);
  leuchten(43);
}

void fuenf() {
  leuchten(29);
  leuchten(30);
  leuchten(31);
  leuchten(32);
}

void zehn() {
  leuchten(22);
  leuchten(23);
  leuchten(24);
  leuchten(25);
}

void viertl() {
  leuchten(12);
  leuchten(13);
  leuchten(14);
  leuchten(15);
  leuchten(16);
  leuchten(17);
}

void halb() {
  leuchten(44);
  leuchten(45);
  leuchten(46);
  leuchten(47);
}

void dreiviertl() {
  leuchten(12);
  leuchten(13);
  leuchten(14);
  leuchten(15);
  leuchten(16);
  leuchten(17);
  leuchten(18);
  leuchten(19);
  leuchten(20);
  leuchten(21);
}

void oise() {
  leuchten(62);
  leuchten(63);
  leuchten(64);
  leuchten(65);
}

void zwoie() {
  leuchten(66);
  leuchten(67);
  leuchten(68);
  leuchten(69);
  leuchten(70);
}

void dreie() {
  leuchten(83);
  leuchten(84);
  leuchten(85);
  leuchten(86);
  leuchten(87);
}

void viere() {
  leuchten(100);
  leuchten(101);
  leuchten(102);
  leuchten(103);
  leuchten(99);
}

void fuenfe() {
  leuchten(49);
  leuchten(50);
  leuchten(51);
  leuchten(52);
  leuchten(53);
}

void sechse() {
  leuchten(58);
  leuchten(59);
  leuchten(60);
  leuchten(61);
  leuchten(62);
  leuchten(63);
}

void siebne() {
  leuchten(104);
  leuchten(105);
  leuchten(106);
  leuchten(107);
  leuchten(108);
  leuchten(109);
}

void achte() {
  leuchten(71);
  leuchten(72);
  leuchten(73);
  leuchten(74);
  leuchten(75);
}

void neune() {
  leuchten(91);
  leuchten(92);
  leuchten(93);
  leuchten(94);
  leuchten(95);
}

void zehne() {
  leuchten(88);
  leuchten(89);
  leuchten(90);
  leuchten(91);
  leuchten(92);
}

void elfe() {
  leuchten(55);
  leuchten(56);
  leuchten(57);
  leuchten(58);
}

void zwoelfe() {
  leuchten(77);
  leuchten(78);
  leuchten(79);
  leuchten(80);
  leuchten(81);
  leuchten(82);
}


void leuchten(int n) {
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


  if (btVal == '0') {
    brightCount++; //zähler für Heligkeitsstufen wird erhöht
    btVal = '9'; //beliebige unbenutzte Zahl, die verhindert, dass mehrere Helligkeitsstufen au einmal durchlaufen werden
    EEPROMWriteBright(brightAddr, brightCount);
  }

  //Heligkeit wird je nach Rest auf elligkeitszähler und 6 verändert
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



  if (btVal == 'a') { //flo
    pixels.clear();
    pixels.setPixelColor(7, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(8, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(9, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(10, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(26, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(27, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(28, pixels.Color(brightness, brightness, brightness));
  }


  if (btVal == 'n') { //noah
    pixels.clear();
    pixels.setPixelColor(7, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(8, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(9, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(10, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(36, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(37, pixels.Color(brightness, brightness, brightness));
  }

  if (btVal == 'N') {
    pixels.clear();
    pixels.setPixelColor(108, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(89, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(86, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(67, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(64, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(45, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(42, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(23, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(20, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(24, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(40, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(48, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(60, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(72, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(80, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(96, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(100, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(97, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(78, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(75, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(56, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(53, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(34, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(31, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(12, pixels.Color(brightness, brightness, brightness));
    pixels.show();
    delay(1000);
    pixels.clear();
    pixels.setPixelColor(17, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(25, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(41, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(45, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(64, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(67, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(85, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(91, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(105, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(104, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(103, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(95, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(79, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(75, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(56, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(53, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(35, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(29, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(15, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(16, pixels.Color(brightness, brightness, brightness));
    pixels.show();
    delay(1000);
    pixels.clear();
    pixels.setPixelColor(108, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(89, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(85, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(68, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(62, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(47, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(39, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(26, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(16, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(28, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(37, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(51, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(58, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(74, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(73, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(72, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(71, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(70, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(69, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(79, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(97, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(100, pixels.Color(brightness, brightness, brightness));
    pixels.show();
    delay(1000);
    pixels.clear();
    pixels.setPixelColor(107, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(90, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(85, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(68, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(63, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(46, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(41, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(24, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(19, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(13, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(30, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(35, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(52, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(57, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(74, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(79, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(96, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(101, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(57, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(58, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(59, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(60, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(61, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(62, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(63, pixels.Color(brightness, brightness, brightness));
    pixels.show();
    delay(1000);
    pixels.clear();
  }

  if (btVal == 't') { //thommy
    pixels.clear();
    pixels.setPixelColor(7, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(8, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(9, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(10, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(38, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(39, pixels.Color(brightness, brightness, brightness));
  }



  if (btVal == 'E') { //elena
    pixels.clear();
    pixels.setPixelColor(107, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(106, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(105, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(104, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(103, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(102, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(101, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(90, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(85, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(68, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(63, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(46, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(41, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(24, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(19, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(18, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(17, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(16, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(15, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(14, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(13, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(62, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(61, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(60, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(59, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(58, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(57, pixels.Color(brightness, brightness, brightness));
    pixels.show();
    delay(1000);
    pixels.clear();
    pixels.setPixelColor(107, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(90, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(85, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(68, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(63, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(46, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(41, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(24, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(19, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(106, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(105, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(104, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(103, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(102, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(101, pixels.Color(brightness, brightness, brightness));
    pixels.show();
    delay(1000);
    pixels.clear();
    pixels.setPixelColor(107, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(106, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(105, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(104, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(103, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(102, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(101, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(90, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(85, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(68, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(63, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(46, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(41, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(24, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(19, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(18, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(17, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(16, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(15, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(14, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(13, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(62, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(61, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(60, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(59, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(58, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(57, pixels.Color(brightness, brightness, brightness));
    pixels.show();
    delay(1000);
    pixels.clear();
    pixels.setPixelColor(108, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(89, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(86, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(67, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(64, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(45, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(42, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(23, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(20, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(24, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(40, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(48, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(60, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(72, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(80, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(96, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(100, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(97, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(78, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(75, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(56, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(53, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(34, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(31, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(12, pixels.Color(brightness, brightness, brightness));
    pixels.show();
    delay(1000);
    pixels.clear();
    pixels.setPixelColor(108, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(89, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(85, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(68, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(62, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(47, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(39, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(26, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(16, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(28, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(37, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(51, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(58, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(74, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(73, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(72, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(71, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(70, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(69, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(79, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(97, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(100, pixels.Color(brightness, brightness, brightness));
    pixels.show();
    delay(1000);
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
    pixels.show();
    delay(1000);
    pixels.clear();
  }

  if (btVal == 'f') { //felix
    pixels.clear();
    pixels.setPixelColor(7, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(8, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(9, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(10, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(96, pixels.Color(brightness, brightness, brightness));
    pixels.setPixelColor(97, pixels.Color(brightness, brightness, brightness));
  }

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
  pixels.show();  //angesteuerte LEDs werden eingeschaltet

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


