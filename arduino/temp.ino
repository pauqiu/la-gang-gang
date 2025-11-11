// Código de Sensor de humedad y temperatura DHT11 con ESP32
#include "DHT.h"

#define DHTPIN 26     // Pin donde está conectado el sensor

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(19200);
  Serial.println("Iniciando...");
  dht.begin();
}
void loop() {
  delay(2000);
  float h = dht.readHumidity(); //Leemos la Humedad
  float t = dht.readTemperature(); //Leemos la temperatura en grados Celsius
  float f = dht.readTemperature(true); //Leemos la temperatura en grados Fahrenheit
  // Lectura de los valores
  Serial.print("Humedad ");
  Serial.print(h);
  Serial.print(" %t");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.println(" *F");
}