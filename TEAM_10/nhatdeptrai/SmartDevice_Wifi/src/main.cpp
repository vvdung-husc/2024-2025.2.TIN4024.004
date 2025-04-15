/*
  ESP32 MQTT Relay Control with Button
  - Controls a relay via MQTT
  - Syncs state between physical button and MQTT
  - Publishes status updates to MQTT topic
*/

#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT broker settings
const char* mqtt_server = "broker.emqx.io";  // Public MQTT broker (change to your broker)
const int mqtt_port = 1883;
const char* mqtt_username = "";  // Leave empty if no authentication
const char* mqtt_password = "";  // Leave empty if no authentication

// MQTT topics
const char* mqtt_topic_sub = "esp32/relay/cmd";     // For receiving commands
const char* mqtt_topic_pub = "esp32/relay/status";  // For publishing status
const char* mqtt_client_id = "ESP32_Relay_Controller";  // Should be unique

// GPIO Pins
const int RELAY_PIN = 5;    // Connected to IN pin of relay module
const int BUTTON_PIN = 19;  // Connected to push button

// Device state variables
bool relayState = false;
bool lastButtonState = HIGH;  // Assumes pull-up configuration
bool currentButtonState;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;  // Debounce time in milliseconds

// MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

// Function prototypes
void setup_wifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void checkButton();
void controlRelay(bool state);
void publishState();

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Configure GPIO pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize relay to OFF state
  digitalWrite(RELAY_PIN, LOW);
  
  // Setup WiFi connection
  setup_wifi();
  
  // Configure MQTT client
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Print basic info
  Serial.println("ESP32 MQTT Relay Controller");
  Serial.println("---------------------------");
}

void loop() {
  // Handle MQTT connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Check physical button state
  checkButton();
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Kết nối tới WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi đã kết nối thành công");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Convert payload to string
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Nhận tin nhắn [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  // Process MQTT command
  if (message.equals("ON")) {
    controlRelay(true);
  } else if (message.equals("OFF")) {
    controlRelay(false);
  } else if (message.equals("TOGGLE")) {
    controlRelay(!relayState);
  } else {
    Serial.println("Lệnh không hợp lệ");
  }
}

void reconnect() {
  // Loop until connected to MQTT broker
  while (!client.connected()) {
    Serial.print("Đang kết nối tới MQTT broker...");
    
    // Attempt to connect with authentication if provided
    bool connected = false;
    if (strlen(mqtt_username) > 0) {
      connected = client.connect(mqtt_client_id, mqtt_username, mqtt_password);
    } else {
      connected = client.connect(mqtt_client_id);
    }
    
    if (connected) {
      Serial.println("đã kết nối");
      
      // Subscribe to command topic
      client.subscribe(mqtt_topic_sub);
      
      // Publish current status
      publishState();
    } else {
      Serial.print("kết nối thất bại, lỗi = ");
      Serial.print(client.state());
      Serial.println(", thử lại sau 5 giây");
      delay(5000);
    }
  }
}

void checkButton() {
  // Read button state with debouncing
  int reading = digitalRead(BUTTON_PIN);
  
  // Check if button state changed
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  // Wait for debounce period
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If button state has truly changed
    if (reading != currentButtonState) {
      currentButtonState = reading;
      
      // If button is pressed (LOW when using INPUT_PULLUP)
      if (currentButtonState == LOW) {
        // Toggle relay state
        controlRelay(!relayState);
        publishState();
      }
    }
  }
  
  lastButtonState = reading;
}

void controlRelay(bool state) {
  // Update relay state
  relayState = state;
  
  // Control relay hardware
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  
  // Log state change
  Serial.print("Relay: ");
  Serial.println(state ? "BẬT" : "TẮT");
}

void publishState() {
  // Publish current state to MQTT
  if (client.connected()) {
    client.publish(mqtt_topic_pub, relayState ? "ON" : "OFF", true);
    Serial.println("Đã đăng trạng thái lên MQTT");
  }
}