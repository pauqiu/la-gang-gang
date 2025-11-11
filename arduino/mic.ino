// Código para medir niveles de sonido con un micrófono analógico en ESP32
const int analogPin = 36;  // ADC0
const int digitalPin = 14; // Pin digital para detección adicional si tienes

unsigned long sampleWindow = 50; // ventana de muestreo en ms
unsigned int sample;

void setup() {
  Serial.begin(19200);
  pinMode(digitalPin, INPUT);
}

void loop() {
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 4095; // ADC 12 bits max

  / t /
      Muestrea durante sampleWindow ms para hallar pico a pico while (
          millis() - startMillis < sampleWindow) {
    sample = analogRead(analogPin);
    if (sample > signalMax) {
      signalMax = sample;
    }
    if (sample < signalMin) {
      signalMin = sample;
    }
  }

  unsigned int peakToPeak = signalMax - signalMin; // amplitud del sonido

  int soundDetected = digitalRead(digitalPin);

  Serial.print("Analog Peak-to-Peak: ");
  Serial.println(peakToPeak);

  Serial.print("Digital: ");
  Serial.println(soundDetected);

  if (soundDetected == HIGH || peakToPeak > 50) { // umbral ajustable
    Serial.println("LOUD SOUND");
  } else {
    Serial.println("LOW SOUND");
  }

  delay(500); // tiempo para evitar saturar el monitor serie
}