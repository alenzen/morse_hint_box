#define PIN_OUT 11

void setup() {
  // Serial setup (optional, can be used for debugging output)
  Serial.begin(115200);

  // input is with pullup resistor, therefore "LOW" on that pin is signal
  pinMode(PIN_OUT, OUTPUT);
  // use builtin LED to visualize what is sent
  pinMode(LED_BUILTIN, OUTPUT);

  // signal HIGH and turn off LED initially
  digitalWrite(PIN_OUT, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
}
