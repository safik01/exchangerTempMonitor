#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Wire.h>
#include <SensirionI2cScd4x.h>
#include <SensirionI2cSht4x.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>

#define POWER_PIN 2
#define PIN_ON 47

GxEPD2_BW<GxEPD2_213_GDEY0213B74, GxEPD2_213_GDEY0213B74::HEIGHT> display(
  GxEPD2_213_GDEY0213B74(5, 17, 16, 4));

SensirionI2cScd4x scd4x;
SensirionI2cSht4x sht4x;

uint16_t co2       = 0;
uint16_t co2Max    = 0;
float    tempIn    = 0;
float    humIn     = 0;
float    tempOut   = 0;
float    humOut    = 0;
float    tempInMax = -99;
float    tempInMin =  99;

void cleanDisplay() {
  for (int i = 0; i < 3; i++) {
    display.setFullWindow();
    display.firstPage();
    do { display.fillScreen(GxEPD_BLACK); } while (display.nextPage());
    delay(500);
    display.setFullWindow();
    display.firstPage();
    do { display.fillScreen(GxEPD_WHITE); } while (display.nextPage());
    delay(500);
  }
}

void drawAll() {
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);

  // === RAMECEK ===
  display.drawRect(0, 0, 250, 122, GxEPD_BLACK);
  display.drawRect(1, 1, 248, 120, GxEPD_BLACK);

  display.drawFastVLine(118, 2, 118, GxEPD_BLACK);
  display.drawFastVLine(119, 2, 118, GxEPD_BLACK);

  display.drawFastHLine(2, 61, 114, GxEPD_BLACK);
  display.drawFastHLine(2, 62, 114, GxEPD_BLACK);

  display.drawFastHLine(120, 61, 128, GxEPD_BLACK);
  display.drawFastHLine(120, 62, 128, GxEPD_BLACK);

  display.drawFastVLine(184, 63, 57, GxEPD_BLACK);
  display.drawFastVLine(185, 63, 57, GxEPD_BLACK);

  // ============================================================
  // === LEVY HORNI — IN teplota ===
  // ============================================================

  // Nadpis "IN"
  display.setFont(NULL);
  display.setTextSize(1);
  display.setCursor(4, 4);
  display.print("IN");

  // Hlavní teplota — FreeSans18pt7b, size 1
  // výška ~25px, baseline y=38 → text od y≈13 do y≈38
  display.setFont(&FreeSans18pt7b);
  display.setTextSize(1);
  char bufIn[6];
  dtostrf(tempIn, 0, 1, bufIn);
  display.setCursor(4, 38);
  display.print(bufIn);

  // Kroužek stupně těsně za číslem
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(bufIn, 7, 38, &x1, &y1, &w, &h);
  int degInX = x1 + w + 3;
  int degInY = y1 + 5;
  display.drawCircle(degInX, degInY, 4, GxEPD_BLACK);
  display.drawCircle(degInX, degInY, 3, GxEPD_BLACK);

  // "C" za kroužkem
  display.setFont(&FreeSans18pt7b);
  display.setTextSize(1);
  display.setCursor(degInX + 10, 38);
  display.print("C");

  // --- max — font NULL size 1, y=50 (pod teplotou) ---
  display.setFont(NULL);
  display.setTextSize(1);

  char bufMax[8];
  dtostrf(tempInMax, 0, 1, bufMax);
  char lineMax[16];
  sprintf(lineMax, "max:%s", bufMax);
  display.setCursor(4, 43);
  display.print(lineMax);
  display.getTextBounds(lineMax, 4, 43, &x1, &y1, &w, &h);
  int maxDegX = x1 + w + 2;
  int maxDegY = y1 + 2;
  display.drawCircle(maxDegX, maxDegY, 2, GxEPD_BLACK);
  display.setCursor(maxDegX + 4, 43);
  display.print("C");

  // --- min — font NULL size 1, y=59 (těsně nad dělící čárou y=61) ---
  char bufMin[8];
  dtostrf(tempInMin, 0, 1, bufMin);
  char lineMin[16];
  sprintf(lineMin, "min:%s", bufMin);
  display.setCursor(4, 53);
  display.print(lineMin);
  display.getTextBounds(lineMin, 4, 53, &x1, &y1, &w, &h);
  int minDegX = x1 + w + 2;
  int minDegY = y1 + 2;
  display.drawCircle(minDegX, minDegY, 2, GxEPD_BLACK);
  display.setCursor(minDegX + 4, 53);
  display.print("C");

  // ============================================================
  // === LEVY DOLNI — OUT teplota ===
  // ============================================================
  display.setFont(NULL);
  display.setTextSize(1);
  display.setCursor(4, 66);
  display.print("OUT");

  display.setFont(&FreeSans12pt7b);
  display.setTextSize(1);
  char bufOut[6];
  dtostrf(tempOut, 0, 1, bufOut);
  display.setCursor(4, 108);
  display.print(bufOut);
  display.getTextBounds(bufOut, 7, 108, &x1, &y1, &w, &h);
  int degOutX = x1 + w + 3;
  int degOutY = y1 + 5;
  display.drawCircle(degOutX, degOutY, 4, GxEPD_BLACK);
  display.drawCircle(degOutX, degOutY, 3, GxEPD_BLACK);
  display.setCursor(degOutX + 11, 108);
  display.print("C");

  // ============================================================
  // === PRAVY HORNI — CO2 ===
  // ============================================================
  display.setFont(NULL);
  display.setTextSize(1);
  display.setCursor(122, 4);
  display.print("CO2");

  char bufCo2Max[14];
  sprintf(bufCo2Max, "max:%d", co2Max);
  display.setCursor(168, 4);
  display.print(bufCo2Max);

  display.setFont(&FreeSans12pt7b);
  display.setTextSize(1);
  display.setCursor(122, 48);
  display.print(co2);
  display.print(" ppm");

  // ============================================================
  // === PRAVY DOLNI — HUM IN + HUM OUT ===
  // ============================================================
  display.setFont(NULL);
  display.setTextSize(1);
  display.setCursor(122, 66);
  display.print("HUM IN");
  display.setCursor(188, 66);
  display.print("HUM OUT");

  display.setFont(&FreeSans12pt7b);
  display.setTextSize(1);
  char bufHumIn[8];
  sprintf(bufHumIn, "%.0f%%", humIn);
  display.setCursor(122, 108);
  display.print(bufHumIn);

  char bufHumOut[8];
  sprintf(bufHumOut, "%.0f%%", humOut);
  display.setCursor(188, 108);
  display.print(bufHumOut);
}

void drawLayout() {
  display.setFullWindow();
  display.firstPage();
  do { drawAll(); } while (display.nextPage());
}

void updateValues() {
  display.setPartialWindow(2, 2, 246, 118);
  display.firstPage();
  do { drawAll(); } while (display.nextPage());
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
  delay(200);

  pinMode(PIN_ON, OUTPUT);
  digitalWrite(PIN_ON, HIGH);
  delay(200);

  display.init(115200, true, 2, false);
  display.setRotation(1);

  cleanDisplay();

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(60, 65);
    display.print("LOADING...");
    display.drawRect(0, 0, 250, 122, GxEPD_BLACK);
  } while (display.nextPage());

  scd4x.begin(Wire, SCD40_I2C_ADDR_62);
  scd4x.stopPeriodicMeasurement();
  delay(500);
  scd4x.startPeriodicMeasurement();

  sht4x.begin(Wire, SHT40_I2C_ADDR_44);

  Serial.println("Pripraveno.");

  drawLayout();
}

void loop() {
  
  
  uint16_t rawCo2;
  float    rawTemp, rawHum;

  int16_t  errScd = scd4x.readMeasurement(rawCo2, rawTemp, rawHum);
  uint16_t errSht = sht4x.measureHighPrecision(tempOut, humOut);

  if (errScd == 0 && rawCo2 != 0 && errSht == 0) {
    co2    = rawCo2;
    tempIn = rawTemp;
    humIn  = rawHum;

    if (co2    > co2Max)    co2Max    = co2;
    if (tempIn > tempInMax) tempInMax = tempIn;
    if (tempIn < tempInMin) tempInMin = tempIn;

    Serial.printf("CO2: %d  TempIN: %.1f  HumIN: %.1f\n", co2, tempIn, humIn);
    Serial.printf("TempOUT: %.1f  HumOUT: %.1f\n", tempOut, humOut);

    updateValues();
  } else {
    if (errScd != 0) Serial.printf("SCD41 error: %d\n", errScd);
    if (errSht != 0) Serial.printf("SHT45 error: %d\n", errSht);
  }

  delay(2000);
}