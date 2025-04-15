#define BLYNK_TEMPLATE_ID "TMPL6YbDEEebI"
#define BLYNK_TEMPLATE_NAME "GIAM SAT NHIP TIM"
#define BLYNK_AUTH_TOKEN "Bba6vzmeMKW2rIBxJFByxHRdmyqty9Po"

#include <Wire.h>
#include <MAX30100_PulseOximeter.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define REPORTING_PERIOD_MS 1000

char ssid[] = "Wokwi-GUEST";
char pass[] = "";
char auth[] = BLYNK_AUTH_TOKEN;

PulseOximeter pox;
uint32_t tsLastReport = 0;

void onBeatDetected() {
  Serial.println("Beat detected");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Blynk.begin(auth, ssid, pass);

  if (!pox.begin()) {
    while (1);
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
  Blynk.run();
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    tsLastReport = millis();
    float bpm = pox.getHeartRate();
    float spo2 = pox.getSpO2();

    Serial.print("Heart rate: ");
    Serial.print(bpm);
    Serial.print(" bpm | SpO2: ");
    Serial.println(spo2);

    Blynk.virtualWrite(V0, bpm);
    Blynk.virtualWrite(V1, spo2);
  }
}