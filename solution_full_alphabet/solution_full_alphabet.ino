#define PIN_OUT 11

// Times defined in milliseconds
#define TIME_UNIT    (250)
#define DOT          TIME_UNIT
#define DASH         (3*TIME_UNIT)
#define SYMBOL_SPACE TIME_UNIT
#define LETTER_SPACE (3 * TIME_UNIT - SYMBOL_SPACE)
#define WORD_SPACE   (7 * TIME_UNIT - LETTER_SPACE)

// Morse code alphabet at https://en.wikipedia.org/wiki/Morse_code
const char *morse_code[] = {
	// a-z
	".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--",
	"-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
	// 0-9
	"-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."
};

// Writes a single symbol, either DOT or DASH
void write_symbol(int symbol_time_ms) {
  digitalWrite(PIN_OUT, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(symbol_time_ms);
  digitalWrite(PIN_OUT, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
  delay(SYMBOL_SPACE);
}

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
