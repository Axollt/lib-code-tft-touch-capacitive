#include <Wire.h>
#include <FT6236.h>
#include <TFT_eSPI.h>

#define TOUCH_INT 4           // Pin interrupt dari FT6236 (pastikan sesuai wiring)
#define RELAY_PIN 17          // Output relay (ganti sesuai kebutuhan)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480

FT6236 ts = FT6236();
TFT_eSPI tft = TFT_eSPI();

bool isOn = false;
bool touched = false;
bool wasTouchedBefore = false;  // Menyimpan status sentuhan sebelumnya

// Tombol di tengah layar
#define BTN_WIDTH 120
#define BTN_HEIGHT 60
int btnX = (SCREEN_WIDTH - BTN_WIDTH) / 2;
int btnY = (SCREEN_HEIGHT - BTN_HEIGHT) / 2;

// Interrupt untuk mendeteksi sentuhan
void IRAM_ATTR touchISR() {
  touched = true;
}

// Gambar tombol
void drawButton(bool state) {
  tft.fillRect(btnX, btnY, BTN_WIDTH, BTN_HEIGHT, state ? TFT_GREEN : TFT_RED);
  tft.drawRect(btnX, btnY, BTN_WIDTH, BTN_HEIGHT, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, state ? TFT_GREEN : TFT_RED);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString(state ? "ON" : "OFF", btnX + BTN_WIDTH / 2, btnY + BTN_HEIGHT / 2);
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(TOUCH_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TOUCH_INT), touchISR, FALLING);

  tft.init();
  tft.setRotation(2);  // Rotasi layar jika diperlukan
  tft.fillScreen(TFT_BLACK);

  if (!ts.begin(40)) {
    Serial.println("FT6236 not found!");
    while (1);
  }

  drawButton(isOn);
  digitalWrite(RELAY_PIN, isOn);
}

void loop() {
  bool isCurrentlyTouched = ts.touched();  // Cek apakah layar disentuh

  if (isCurrentlyTouched && !wasTouchedBefore) {
    // Sentuhan baru terdeteksi (rising edge)
    TS_Point p = ts.getPoint();

    // Kalibrasi manual
    int mappedX = map(p.x, 4, 302, 0, SCREEN_WIDTH);
    int mappedY = map(p.y, 0, 478, 0, SCREEN_HEIGHT);

    Serial.printf("Touched at raw x=%d, y=%d => mapped x=%d, y=%d\n", p.x, p.y, mappedX, mappedY);

    if (mappedX >= btnX && mappedX <= (btnX + BTN_WIDTH) &&
        mappedY >= btnY && mappedY <= (btnY + BTN_HEIGHT)) {
      isOn = !isOn;
      drawButton(isOn);
      digitalWrite(RELAY_PIN, isOn);
    }
  }

  wasTouchedBefore = isCurrentlyTouched;  // Simpan status untuk perbandingan loop berikutnya
  delay(10);  // Loop stabil
}
