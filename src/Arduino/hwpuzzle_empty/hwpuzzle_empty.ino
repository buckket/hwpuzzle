#include <HardwareSerial.h>

void setup() {
  // Set Reset as INPUT
  pinMode(PIN_PA2, INPUT);
  
  // Init Serial
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.write("Up and running!\n");
}

void loop() {
   if (digitalRead(PIN_PA2) == LOW)  {
    Serial.print("RESET detected\n");
    delay(500);
  }
}
