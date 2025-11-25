// Código de Sensor ultrasonico y sensor de tilt con ESP32
#include <WiFi.h>

// Configurar WiFi
const char *wifi_ssid = "Lab-3-5";       // Red WiFi
const char *wifi_password = "Cata2960!"; // Password

// Servidor receptor
const char *host = "10.1.137.72"; // Example: "10.1.35.12"
const uint16_t port =
    9090; // PORT INTERNET DE SERVIDOR RECEPTOR - como 9080, 9090...

// Ultrasonic sensor: Distance (cm) detected of distance
uint16_t cm = 0;
// Tilt sensor: High = 1, Low = 0
boolean tiltState = 0;

// Pines del sensor
const int triggerPin = 14; // Trigger pin
const int echoPin = 27;    // Echo pin
const int tiltPin = 13;    // Tilt pin

const uint16_t SENSOR_ID = 22;
const int ARRAY_SIZE =
    3; // Tamaño de la cantidad de datos del array que se envía a receptor

WiFiClient client;

void connectWiFi() {
  Serial.print("Conectando a ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);

  // Esperar hasta que la conexión esté establecida
  while (WiFi.status() != WL_CONNECTED) {
    // Esperar 500 ms y volver a comprobar (Imprimir un punto cada vez)
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
  // Establece el pin de activación en estado ALTO durante 10 microsegundos
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  float duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.0343 / 2;
  // Lee el pin de eco y devuelve el tiempo de viaje de la onda de sonido en
  // microsegundos * 0.01723
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

void setup() {
  Serial.begin(19200); // Cambiarlo según sea necesario (19200, 9600, etc.)
  connectWiFi();
}

void loop() {
  cm = readUltrasonicDistance(triggerPin, echoPin); // Determinar la distancia
  Serial.println("Distancia: ");
  Serial.print(cm);
  Serial.println(" cm");

  uint16_t tiltValue = readTilt();
  Serial.print("Tilt: ");
  Serial.println(tiltValue);

  // Enviar datos al servidor
  if (client.connect(host, port)) {
    uint16_t dataMessage[ARRAY_SIZE];
    dataMessage[0] = SENSOR_ID;
    dataMessage[1] = cm;
    dataMessage[2] = tiltValue;
    size_t bytesEnviados = client.write((const uint8_t *)dataMessage,
                                        ARRAY_SIZE * sizeof(uint16_t));
    Serial.println("Datos enviados - Bytes ");
    Serial.println(bytesEnviados);

    client.stop();
  } else {
    Serial.println("No se pudo conectar al servidor");
  }

  delay(15000); // Esperar 15 segundos (Cambiar según sea necesario)
}
