
#include <main.h>

#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels

#define OLED_SDA 13
#define OLED_SCL 12

#define DHTPIN 16     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22 // DHT 22 (AM2302)

#define LED_GREEN 15
#define LED_YELLOW 2
#define LED_RED 4
// #define LED_CAM     5

////**************** VVDUNG ********** */
// create an OLED display object connected to I2C
MainApp::MainApp()
{
  ulTimer = 0;
  curLed = NULL;
}
MainApp::~MainApp()
{
}
void MainApp::setup(int pinGreen, int pinYellow, int pinRed)
{
  ledGreen.setup(pinGreen);
  ledYellow.setup(pinYellow);
  ledRed.setup(pinRed);
}
void MainApp::run(DHT &dht, Adafruit_SSD1306 &oled)
{
  static float prevTemp = 0.0;
  static bool ledStatus = false;

  if (!IsReady(ulTimer, 500))
    return;
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  

  if (prevTemp != t)
  {
    prevTemp = t;
    if (curLed) curLed->set(false);
    ledStatus = true;
    String strTemp = String("Temperature: ");
    if (t < 13.0)
    {
      strTemp += "TOO COLD";
      curLed = &ledGreen;
    }
    else if (t < 20.0)
    {
      if (curLed)
        curLed->set(false);
      strTemp += "COLD";
      curLed = &ledGreen;
    }
    else if (t < 25.0)
    {
      if (curLed)
        curLed->set(false);
      strTemp += "COOL";
      curLed = &ledYellow;
    }
    else if (t < 30.0)
    {
      if (curLed)
        curLed->set(false);
      strTemp += "WARN";
      curLed = &ledYellow;
    }
    else if (t < 35.0)
    {
      if (curLed)
        curLed->set(false);
      strTemp += "HOT";
      curLed = &ledRed;
    }
    else
    {
      if (curLed)
        curLed->set(false);
      strTemp += "TOO HOT";
      curLed = &ledRed;
    }

    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.print(strTemp.c_str());
    oled.setTextSize(2);
    oled.setCursor(0, 12);
    oled.print(t);
    oled.print(" ");
    oled.setTextSize(1);
    oled.cp437(true);
    oled.write(167); //*C
    oled.setTextSize(2);
    oled.print("C");

    oled.setTextSize(1);
    oled.setCursor(0, 34);
    oled.print("Humidity: ");
    oled.setTextSize(2);
    oled.setCursor(0, 46);
    oled.print(h);
    oled.print(" %");

    oled.display();
    return;
  }

  // Turn ON the corresponding LED
  if (curLed != NULL)
  {
    curLed->set(ledStatus);
    ledStatus = !ledStatus;
  }
}

MainApp app;

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DHT dht(DHTPIN, DHTTYPE);

String strTemp;

void setup()
{
  // put your setup code here, to run once:

  app.setup(LED_GREEN, LED_YELLOW, LED_RED);

  dht.begin();

  TwoWire *_Wire = &Wire;
  _Wire->setPins(OLED_SDA, OLED_SCL);
  // initialize OLED display with I2C address 0x3C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1)
      ;
  }

  delay(1000);         // wait two seconds for initializing
  oled.clearDisplay(); // clear display

  oled.setTextSize(2);               // set text size
  oled.setTextColor(WHITE);          // set text color
  oled.setCursor(0, 2);              // set position to display (x,y)
  oled.println("   IOT\n Welcome!"); // set text
  oled.display();                    // display on OLED
  delay(2000);
}
void loop()
{
  // put your main code here, to run repeatedly:
  app.run(dht, oled);
}
