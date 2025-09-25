// Signal K + Bilge Water Height Sensor
//
// Based on SensESP template + JSN-SR04T depth code (median + EMA smoothing)

#include <memory>
#include "sensesp.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

// ==== Bilge water height sensor settings ====
#define TRIG_PIN 26   // GPIO26 (left side, D0, just above 25)
#define ECHO_PIN 25   // GPIO25 (left side, D2, bottom)

#define SOUND_SPEED 0.0343f   // cm/us
#define MAX_DISTANCE 400      // cm
#define MIN_DISTANCE 20       // cm

#define EMA_ALPHA 0.3f
#define MEDIAN_WINDOW 7

float emaDistance = 0.0f;
float referenceHeight = 100.0f;  // distance from sensor to bilge bottom in cm
float medianBuf[MEDIAN_WINDOW];
uint8_t medianIdx = 0;
bool bufferFilled = false;

float getMedian() {
  float buf[MEDIAN_WINDOW];
  memcpy(buf, medianBuf, sizeof(buf));
  for (int i = 0; i < MEDIAN_WINDOW - 1; i++) {
    for (int j = i + 1; j < MEDIAN_WINDOW; j++) {
      if (buf[j] < buf[i]) {
        float tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
      }
    }
  }
  return buf[MEDIAN_WINDOW / 2];
}

// Function to read height above bilge water (cm)
float readBilgeHeightCM() {
  // Trigger sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo time
  long duration = pulseIn(ECHO_PIN, HIGH, MAX_DISTANCE * 58);
  float distance = (duration > 0) ? duration * SOUND_SPEED / 2.0f : NAN;

  if (isnan(distance) || distance < MIN_DISTANCE || distance > MAX_DISTANCE) {
    return NAN;
  }

  // Store into buffer
  medianBuf[medianIdx++] = distance;
  if (medianIdx >= MEDIAN_WINDOW) {
    medianIdx = 0;
    bufferFilled = true;
  }

  if (bufferFilled) {
    float median = getMedian();
    emaDistance = EMA_ALPHA * median + (1.0f - EMA_ALPHA) * emaDistance;
    float heightAboveWater = emaDistance;  // measured distance to water surface
    return heightAboveWater;
  } else {
    return distance;  // rough value until filter fills
  }
}

// ==== SensESP setup ====
void setup() {
  SetupLogging(ESP_LOG_DEBUG);

  // Initialize IO pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Construct the global SensESPApp
  SensESPAppBuilder builder;
  sensesp_app = (&builder)
                    ->set_hostname("sensesp-bilge-sensor")
                    ->get_app();

  // Bilge water height RepeatSensor every 1s
  const unsigned int bilgeInterval = 1000;
  auto bilge_sensor = std::make_shared<RepeatSensor<float>>(
      bilgeInterval, []() { return readBilgeHeightCM(); });

  // Signal K metadata - centimeters
  auto bilge_metadata =
      std::make_shared<SKMetadata>("cm", "Height above bilge water");

  // Publish to Signal K
  auto bilge_sk_output = std::make_shared<SKOutput<float>>(
      "environment.bilge.height",      // SK path
      "/Sensors/Bilge/HeightAbove",    // config path in UI
      bilge_metadata);

  bilge_sensor->connect_to(bilge_sk_output);

  // Prevent objects from being garbage collected
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
