# Sensor LDR luz
int valorLDR = 0;   // Variable de tipo número entero para almacenar los datos recogidos del sensor analógico LDR (Light Dependant Resitor). 
int pinLDR = 35;    // Pin analógico

void setup() { 
Serial.begin(19200);
}

void loop() { 
valorLDR = analogRead(pinLDR);
Serial.println(valorLDR);      // Imprimir el valor
delay(1000);
}
