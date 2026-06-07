#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// Set your desired Hotspot Name and Password
const char* ssid = "ESP32 hotspot";
const char* password = "password123"; // Password MUST be at least 8 characters!

const int ledPin = 2;
const int pwmChannel = 0;

WebServer server(80);

void handleRoot() {
  String page = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial; text-align: center; margin-top: 50px; }
      input[type=range] { width: 80%; max-width: 400px; margin-top: 20px; }
      h2 { color: #333; }
    </style>
  </head>
  <body>

  <h2>ESP32 Hotspot Control</h2>

  <input type="range"
         min="0"
         max="255"
         value="0"
         oninput="updateBrightness(this.value)">

  <p>Brightness: <span id="value" style="font-weight:bold;">0</span></p>

  <script>
    let timeout;
    function updateBrightness(val) {
      document.getElementById("value").innerHTML = val;
      
      // Debounce: Wait 50ms before sending to prevent crashing the ESP32
      clearTimeout(timeout);
      timeout = setTimeout(() => {
        fetch("/set?brightness=" + val);
      }, 50);
    }
  </script>

  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", page);
}

void handleBrightness() {
  if (server.hasArg("brightness")) {
    int brightness = server.arg("brightness").toInt();
    ledcWrite(pwmChannel, brightness);
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Initialize PWM for the LED
  ledcSetup(pwmChannel, 5000, 8);
  ledcAttachPin(ledPin, pwmChannel);

  Serial.println("\n--- Starting ESP32 Hotspot ---");

  // Start the Access Point (Hotspot)
  WiFi.softAP(ssid, password);

  // Get and print the default IP address of the ESP32 AP
  IPAddress IP = WiFi.softAPIP();
  
  Serial.print("Hotspot Ready! Connect your phone/PC to WiFi: ");
  Serial.println(ssid);
  Serial.print("Then open your browser and go to: http://");
  Serial.println(IP);

  // Route the web pages
  server.on("/", handleRoot);
  server.on("/set", handleBrightness);

  server.begin();
}

void loop() {
  server.handleClient();
}