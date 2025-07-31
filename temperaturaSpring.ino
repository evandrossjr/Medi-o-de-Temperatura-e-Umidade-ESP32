#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include "secrets.h"  


// ========== CONFIGURAÇÕES DO DISPLAY OLED ==========
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ========== CONFIGURAÇÕES DO DHT11 ==========
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ========== CONFIGURAÇÕES DO BUZZER ==========
#define BUZZER_PIN 13
unsigned long previousBipTime = 0;
const int intervaloBip = 120000; // 2 minutos

void setup() {
  Serial.begin(115200);

  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado com sucesso!");

  // Inicia display OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erro ao iniciar o display OLED");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Sistema Iniciado");
  display.display();

  // Inicia sensor e buzzer
  dht.begin();
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  delay(2000);
}

void loop() {
  float temp = dht.readTemperature();
  float umid = dht.readHumidity();

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  if (WiFi.status() == WL_CONNECTED) {
    display.println("Wi-Fi: Conectado");
  } else {
    display.println("Wi-Fi: OFFLINE");
  }

  if (isnan(temp) || isnan(umid)) {
    Serial.println("Erro na leitura do DHT11");
    display.setCursor(0, 15);
    display.println("Erro sensor DHT11");
    display.display();
    delay(2000);
    return;
  }

  // Log de depuração no Serial Monitor
  Serial.println("====== LEITURA ATUAL ======");
  Serial.print("TEMPERATURA lida: ");
  Serial.println(temp);
  Serial.print("UMIDADE lida: ");
  Serial.println(umid);

  // Mostra no display
  display.setCursor(0, 15);
  display.setTextSize(2);
  display.print(temp, 1);
  display.print(" C");

  display.setCursor(0, 40);
  display.setTextSize(2);
  display.print(umid, 0);
  display.print(" %");
  display.display();

  // Alarme se temperatura > 35 °C
  if (temp > 35) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousBipTime >= intervaloBip) {
      tone(BUZZER_PIN, 1000, 500); // 1000 Hz por 500 ms
      previousBipTime = currentMillis;
    }
  }

  enviarParaBackend(temp, umid);

  delay(2000);
}

void enviarParaBackend(float temperatura, float umidade) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // JSON a ser enviado
    String json = "{\"temperatura\": " + String(temperatura, 1) +
                  ", \"umidade\": " + String(umidade, 1) + "}";

    Serial.println("====== ENVIO PARA BACKEND (Atual) ======");
    Serial.print("JSON gerado: ");
    Serial.println(json);

    // Enviar para endpoint de temperatura atual
    http.begin(urlBackend);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(json);
    Serial.print("Resposta HTTP: ");
    Serial.println(httpResponseCode);
    http.end();

    delay(100); // pequena pausa

    // Enviar para endpoint do histórico
    Serial.println("====== ENVIO PARA BACKEND (Histórico) ======");
    Serial.print("JSON gerado: ");
    Serial.println(json);

    http.begin(urlBackHistorico);
    http.addHeader("Content-Type", "application/json");
    httpResponseCode = http.POST(json);
    Serial.print("Resposta HTTP: ");
    Serial.println(httpResponseCode);
    http.end();

  } else {
    Serial.println("Wi-Fi desconectado. Dados não enviados.");
  }
}



