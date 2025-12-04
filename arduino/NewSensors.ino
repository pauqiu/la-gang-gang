#include <WiFi.h>

// Configurar WiFi
const char *wifi_ssid = "Lab-3-5";       // Red WiFi
const char *wifi_password = "Cata2960!"; // Password

// Servidor receptor
const char *host = "10.1.35.14"; // Example: "10.1.35.12"
const uint16_t port = 9090; // PORT INTERNET DE SERVIDOR RECEPTOR

// Pines del sensor ultrasónico
const int triggerPin = 14; // Trigger pin
const int echoPin = 27;    // Echo pin
const int tiltPin = 13;    // Tilt pin

// Pines del micrófono
const int analogPin = 34;  // ADC0
const int digitalPin = 12; // Pin digital para detección adicional

// Pin Luz LDR
const int ldrPin = 4;

// Variables de sensores
uint16_t cm = 0; // Distancia en cm
boolean tiltState = 0; // Estado del tilt
unsigned int soundLevel = 0; // Nivel de sonido
int soundDetected = 0; // Detección digital de sonido
int lightLevel = 0;

// Configuración micrófono
unsigned long sampleWindow = 50; // ventana de muestreo en ms
unsigned int sample;

const uint16_t SENSOR_ID = 22;
const int ARRAY_SIZE = 5; //5; // ID, Distancia, tilt, sonido, luz

WiFiClient client;

void connectWiFi() {
  Serial.print("Conectando a ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);

  // Esperar hasta que la conexión esté establecida
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n WiFi conectado");
  Serial.print("IP asignada: ");
  Serial.println(WiFi.localIP());
}

long readUltrasonicDistance(int triggerPin, int echoPin) {
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  float duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.0343 / 2;
  return distance;
}

uint16_t readTilt() {
  pinMode(tiltPin, INPUT);
  tiltState = digitalRead(tiltPin);
  Serial.print("Tilt: ");
  if (tiltState == HIGH) {
    Serial.println("HIGH");
    return 1;
  } else {
    Serial.println("LOW");
    return 0;
  }
}

unsigned int readSoundLevel() {
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 4095; // ADC 12 bits max

  // Muestrea durante sampleWindow ms para hallar pico a pico
  while (millis() - startMillis < sampleWindow) {
    sample = analogRead(analogPin);
    if (sample > signalMax) {
      signalMax = sample;
    }
    if (sample < signalMin) {
      signalMin = sample;
    }
  }

  unsigned int peakToPeak = signalMax - signalMin; // amplitud del sonido
  return peakToPeak;
}

uint16_t readDigitalSound() {
  return digitalRead(digitalPin);
}

uint16_t readLightLevel() {
  int rawValue = analogRead(ldrPin);
  Serial.println(rawValue);
  return rawValue;
}

void setup() {
  Serial.begin(19200);
  
  // Configurar pines
  pinMode(digitalPin, INPUT);
  pinMode(tiltPin, INPUT);
  
  connectWiFi();
}

void loop() {
  // Leer sensores
  cm = readUltrasonicDistance(triggerPin, echoPin);
  uint16_t tiltValue = readTilt();
  soundLevel = readSoundLevel();
  soundDetected = readDigitalSound();
  lightLevel = readLightLevel();

  // Mostrar lecturas en monitor serie
  Serial.println("=== LECTURAS DE SENSORES ===");
  Serial.print("Distancia: ");
  Serial.print(cm);
  Serial.println(" cm");
  
  Serial.print("Tilt: ");
  Serial.println(tiltValue);
  
  Serial.print("Nivel de sonido (Analog): ");
  Serial.println(soundLevel);

  Serial.print("Luz (LDR): ");
  Serial.println(lightLevel);

  Serial.println("=============================");

  // Enviar datos al servidor
  if (client.connect(host, port)) {
    uint16_t dataMessage[ARRAY_SIZE];
    dataMessage[0] = SENSOR_ID;
    dataMessage[1] = cm;
    dataMessage[2] = tiltValue;
    dataMessage[3] = soundLevel; // Enviar nivel de sonido analógico
    dataMessage[4] = lightLevel;
    
    size_t bytesEnviados = client.write((const uint8_t *)dataMessage,
                                        ARRAY_SIZE * sizeof(uint16_t));
    Serial.print("Datos enviados - Bytes: ");
    Serial.println(bytesEnviados);

    client.stop();
  } else {
    Serial.println("No se pudo conectar al servidor");
  }

  delay(15000); // Esperar 15 segundos
}
