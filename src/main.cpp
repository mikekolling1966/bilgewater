#include <M5Stack.h>

/* Wiring
   JSN 5V  -> M5 5V
   JSN GND -> M5 GND
   JSN TRIG -> GPIO26 (output)
   JSN ECHO -> GPIO16 (input) THROUGH DIVIDER: 10k to ECHO, 20k to GND
*/

static const int PIN_TRIG = 26;     // TRIG (output)
static const int PIN_ECHO = 16;     // ECHO (input via 10k/20k divider)  <-- moved off GPIO25 (speaker)
static const float TANK_HEIGHT_CM = 80.0f;
static const float BLIND_CM       = 25.0f;
static const float MIN_CM         = 25.0f;
static const float MAX_CM         = 600.0f;

inline float speedOfSound_cm_per_us(float tC) {
  return (331.4f + 0.6f * tC) * 100.0f / 1e6f;
}

float readOnceCm(float tempC = 20.0f) {
  digitalWrite(PIN_TRIG, LOW); delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);  // shorter pulse
  digitalWrite(PIN_TRIG, LOW);
  unsigned long dur = pulseIn(PIN_ECHO, HIGH, 60000UL); // up to 60ms
  if (dur == 0) return NAN;
  return (dur * speedOfSound_cm_per_us(tempC)) / 2.0f;
}

float readMedianCm(int n=5) {
  float v[11]; if (n>11) n=11; if (n<3) n=3; if (!(n&1)) n++; // force odd 3..11
  int m=0;
  for (int i=0;i<n;i++){
    float d = readOnceCm(20.0f);
    if (!isnan(d) && d>=MIN_CM && d<=MAX_CM) v[m++]=d;
    delay(30);
  }
  if (m==0) return NAN;
  // insertion sort small m
  for (int i=1;i<m;i++){ float x=v[i]; int j=i-1; while(j>=0 && v[j]>x){v[j+1]=v[j]; j--; } v[j+1]=x; }
  return v[m/2];
}

void setup() {
  M5.begin();
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  M5.Lcd.setTextSize(2);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.println("Ultrasonic Tank Level");
  M5.Lcd.println("TRIG=26  ECHO=16 (div)");
}

void loop() {
  // ping less often to reduce audible clicking
  float d_raw = readMedianCm(5);

  // peg near-full inside blind zone
  float d_eff = d_raw;
  bool inBlind = false;
  if (!isnan(d_eff) && d_eff < BLIND_CM) { d_eff = BLIND_CM; inBlind = true; }

  float depth = NAN;
  if (!isnan(d_eff)) {
    depth = TANK_HEIGHT_CM - d_eff;
    if (depth < 0) depth = 0;
  }

  M5.Lcd.setCursor(0,0);
  M5.Lcd.printf("Distance: %6.1f cm   \n", isnan(d_raw)?-1.0f:d_raw);
  M5.Lcd.printf("Depth   : %6.1f cm   \n", isnan(depth)?-1.0f:depth);
  M5.Lcd.printf("Blind   : %4.0f cm %s\n", BLIND_CM, inBlind?"(pegged)":"     ");

  delay(1000);  // 1 second between pings = much quieter
}
