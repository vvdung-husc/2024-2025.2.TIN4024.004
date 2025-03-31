#include <Arduino.h>

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/telegram-esp32-motion-detection-arduino/
  
  Project created using Brian Lough's Universal Telegram Bot Library: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Initialize Telegram BOT
#define BOTtoken "7724534897:AAEoPPPu3U9kTZ7_pE6rpL6Us0Nd-EfuypI"  // your Bot Token (Get from Botfather)
#define CHAT_ID "1333147355"
// Dùng ChatGPT để nhờ hướng dẫn tìm giá trị GROUP_ID này
#define GROUP_ID "-2697080811" //thường là một số âm

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int motionSensor = 27; // PIR Motion Sensor
bool motionDetected = false;
int ledPin = 22;
bool ledState = LOW;
int bot_delay = 1000;
unsigned long lastTimeBotRan;

void handleNewMessages(int numNewMessages) {
  Serial.println("Handling New Message");
  Serial.println(String(numNewMessages));
  for (int i=0; i<numNewMessages; i++) {
    // Chat ID của yêu cầu
    String chat_id = String(bot.messages[i].chat_id);
    Serial.println(bot.messages[i].chat_id);
    if (chat_id != GROUP_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // In ra tin nhắn nhận được
    String user_text = bot.messages[i].text;
    Serial.println(user_text);
    String your_name = bot.messages[i].from_name;
    if (user_text == "/start") {
      String welcome = "Welcome, " + your_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "Send /led2_on to turn GPIO2 ON \n";
      welcome += "Send /led2_off to turn GPIO2 OFF \n";
      welcome += "Send /get_state to request current GPIO state \n";
      bot.sendMessage(chat_id, welcome, "");
    }
    if (user_text == "/led2_on") {
      bot.sendMessage(chat_id, "LED state set to ON", "");
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
    }
    
    if (user_text == "/led2_off") {
      bot.sendMessage(chat_id, "LED state is set to OFF", "");
      ledState = LOW;
      digitalWrite(ledPin, ledState);
    }
    
    if (user_text == "/get_state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "LED is ON", "");
      }
      else{
        bot.sendMessage(chat_id, "LED is OFF", "");
      }
    }
  }
}
//Định dạng chuỗi %s,%d,...
String StringFormat(const char* fmt, ...){
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

// Indicates when motion is detected
void IRAM_ATTR detectsMovement() {
  //Serial.println("MOTION DETECTED!!!");
  motionDetected = true;
}

void setup() {
  Serial.begin(115200);

  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  
  bot.sendMessage(GROUP_ID, "IoT Developer started up");
}


void loop() {
  static uint count_ = 0;

  if(motionDetected){
    ++count_;
    Serial.print(count_);Serial.println(". MOTION DETECTED => Waiting to send to Telegram");    
    String msg = StringFormat("%u => Motion detected!",count_);
    bot.sendMessage(GROUP_ID, msg.c_str());
    Serial.print(count_);Serial.println(". Sent successfully to Telegram: Motion Detected");
    motionDetected = false;
  }

  if (millis() > lastTimeBotRan + bot_delay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      Serial.println("Got Response!");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}