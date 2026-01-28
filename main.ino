#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// 1. WIFI DATA
const char* ssid = "";
const char* password = "";

// 2. PINS (Check these holes!)
#define SS_PIN    5
#define RST_PIN   22 
#define SERVO_PIN 13
#define LED       12 

MFRC522 rfid(SS_PIN, RST_PIN);
Servo myServo;
WebServer server(80);
bool isOpen = false;

void setup() {
  Serial.begin(115200);
  delay(1000); // Give it a second to breathe
  
  pinMode(LED, OUTPUT);
  myServo.attach(SERVO_PIN);
  myServo.write(0);
  
  SPI.begin();
  rfid.PCD_Init(); 
  
  // IF YOU SEE THIS IN SERIAL MONITOR, THE HARDWARE IS GOOD:
  Serial.println("--- READER ONLINE ---");

  WiFi.begin(ssid, password);
  server.on("/", [](){ 
    String html = "<html><body style='background:#000;color:#39FF14;text-align:center;font-family:monospace;'>";
    html += "<h1>ARCADE ACCESS</h1>";
    html += "<h2>STATUS: " + String(isOpen ? "OPEN" : "LOCKED") + "</h2>";
    html += "<div style='font-size:50px;'>" + String(isOpen ? "🐈" : "😿") + "</div>";
    html += "<br><a href='/t' style='color:#FF00FF;'>[ OVERRIDE ]</a>";
    html += "<script>setInterval(function(){location.reload();}, 2000);</script></body></html>";
    server.send(200, "text/html", html); 
  });
  
  server.on("/t", [](){ 
    isOpen = !isOpen; myServo.write(isOpen ? 90 : 0); 
    server.sendHeader("Location", "/"); server.send(303); 
  });
  
  server.begin();
}

void loop() {
  server.handleClient();
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  digitalWrite(LED, HIGH); delay(100); digitalWrite(LED, LOW);
  isOpen = !isOpen;
  myServo.write(isOpen ? 90 : 0);
  Serial.println("BADGE DETECTED");
  delay(1000);
  rfid.PICC_HaltA();
}
