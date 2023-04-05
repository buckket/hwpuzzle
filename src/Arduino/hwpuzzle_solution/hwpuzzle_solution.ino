#include <HardwareSerial.h>

#define ENABLE_OUTPUTS
#define DELAY 18

#define STATE_READ 0
#define STATE_WRITE 1
#define STATE_FINISHED 2

int STATE = 0;
int BUFFER[100] = {0};
int TO_READ = 1;
int TO_WRITE = 0;

void setup() {
  // Set Reset as INPUT
  pinMode(PIN_PA2, INPUT);

  // Set LED0-4 as INPUT
  pinMode(PIN_PC0, INPUT);
  pinMode(PIN_PC1, INPUT);
  pinMode(PIN_PC2, INPUT);
  pinMode(PIN_PC3, INPUT);

#ifdef ENABLE_OUTPUTS
  //Set INPUT0-4 as OUTPUT
  pinMode(PIN_PA3, OUTPUT);
  pinMode(PIN_PA4, OUTPUT);
  pinMode(PIN_PA5, OUTPUT);
  pinMode(PIN_PA6, OUTPUT);
  pinMode(PIN_PA7, OUTPUT);
#endif

  // Write HIGH (active-low)
  digitalWrite(PIN_PA3, HIGH);
  digitalWrite(PIN_PA4, HIGH);
  digitalWrite(PIN_PA5, HIGH);
  digitalWrite(PIN_PA6, HIGH);
  digitalWrite(PIN_PA7, HIGH);

  // Init Serial
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.write("Up and running!\n");
}

void toggle_pin(int pin) {
  digitalWrite(pin, LOW);
  delay(DELAY);
  digitalWrite(pin, HIGH);
  delay(DELAY);
}

void decode_message() {
  Serial.printf("Message length: %d\n", TO_WRITE);

  for (int i = 0; i < TO_WRITE / 4; i++) {
    int x = 0;
    x += 1 * BUFFER[i * 4 + 0];
    x += 4 * BUFFER[i * 4 + 1];
    x += 16 * BUFFER[i * 4 + 2];
    x += 64 * BUFFER[i * 4 + 3];
    Serial.print((char)x);
  }
}

int read_pins() {
  int led_0 = !digitalRead(PIN_PC0);
  int led_1 = !digitalRead(PIN_PC1);
  int led_2 = !digitalRead(PIN_PC2);
  int led_3 = !digitalRead(PIN_PC3);

  int sum = led_0 + led_1 + led_2 + led_3;
  if (sum == 2) {
    return 4;
  } else if (sum == 4) {
    return 5;
  }


  if (led_0 > 0) return 0;
  if (led_1 > 0) return 1;
  if (led_2 > 0) return 2;
  if (led_3 > 0) return 3;

  return -1;
}

void loop() {
  if (digitalRead(PIN_PA2) == LOW)  {
    TO_READ = 1;
    TO_WRITE = 0;
    STATE = STATE_READ;
    toggle_pin(PIN_PA3);
    Serial.print("RESET detected\n");
    delay(500);
  }
  switch (STATE) {
    case STATE_READ:
      toggle_pin(PIN_PA3);
      for (int i = 0; i < TO_READ; i++) {
        int input = read_pins();
        if (input >= 0 && input < 4) {
          BUFFER[i] = input;
          Serial.printf("Detected %d\n", input);
          toggle_pin(PIN_PA3);
          continue;
        }
        if (input == 4) {
          STATE = STATE_FINISHED;
          Serial.print("We are finished, decoding input\n");
          decode_message();
          break;
        }
        if (input == 5) {
          Serial.print("There has been an error, retrying\n");
          TO_READ = 1;
          TO_WRITE = 0;
          toggle_pin(PIN_PA3);
          break;
        }
        if (input == -1) {
          Serial.print("NO LED active, try writing\n");
          TO_READ++;
          TO_WRITE++;
          STATE = STATE_WRITE;
          toggle_pin(PIN_PA3);
          break;
        }
      }
      break;
    case STATE_WRITE:
      for (int i = 0; i < TO_WRITE; i++) {
        Serial.printf("Writing %d\n", BUFFER[i]);
        if (BUFFER[i] == 0) toggle_pin(PIN_PA4);
        if (BUFFER[i] == 1) toggle_pin(PIN_PA5);
        if (BUFFER[i] == 2) toggle_pin(PIN_PA6);
        if (BUFFER[i] == 3) toggle_pin(PIN_PA7);
      }
      STATE = STATE_READ;
      break;
    case STATE_FINISHED:
      toggle_pin(PIN_PA3);
      delay(100);
      break;
  }
}
