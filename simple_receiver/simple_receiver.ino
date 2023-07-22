#define PIN_IN 12

// Times defined in milliseconds
#define TIME_UNIT          (250)
#define DOT                TIME_UNIT
#define DASH               (3*TIME_UNIT)
#define SYMBOL_SPACE       TIME_UNIT
#define RECV_LETTER_SPACE  (3 * TIME_UNIT)
#define RECV_WORD_SPACE    (7 * TIME_UNIT)

#define ERASE_TIMEOUT      (20 * TIME_UNIT)
#define LETTER_BUFFER_SIZE  8
#define WORD_BUFFER_SIZE   10

#define SYMBOL_DOT         '.'
#define SYMBOL_DASH        '-'

#define MORSE_CODE_LENGTH 36
const char *morse_code[MORSE_CODE_LENGTH] = {
  // a-z
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--",
  "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
  // 0-9
  "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."
};

volatile unsigned long last_low_millis = 0;
volatile unsigned long last_high_millis = 0;
volatile char word_buffer[WORD_BUFFER_SIZE];
volatile char letter_buffer[LETTER_BUFFER_SIZE];
volatile byte letter_buffer_index = 0;
volatile byte word_buffer_index = 0;
volatile bool word_complete = false;

void parse_letter_buffer() {
  for (int i = 0; i < MORSE_CODE_LENGTH; ++i) {
    if (strcmp(morse_code[i], letter_buffer) == 0) {
      if (i < ('z' - 'a')) {
        word_buffer[word_buffer_index] = 'a' + i;
      } else {
        word_buffer[word_buffer_index] = i - ('z' - 'a' + 1) + '0';
      }
      //Serial.println(word_buffer[word_buffer_index]);
      word_buffer_index += 1;
      if (word_buffer_index >= WORD_BUFFER_SIZE) {
        word_complete = true;
      }
    }
  }
  for (int i = 0; i < LETTER_BUFFER_SIZE; ++i) letter_buffer[i] = 0;
  letter_buffer_index = 0;
}

void flush_buffers() {
  for (int i = 0; i < LETTER_BUFFER_SIZE; ++i) letter_buffer[i] = 0;
  for (int i = 0; i < WORD_BUFFER_SIZE; ++i) word_buffer[i] = 0;
  letter_buffer_index = 0;
  word_buffer_index = 0;
  word_complete = false;
}

void read_morse() {
  if (digitalRead(PIN_IN) == HIGH) {
    digitalWrite(LED_BUILTIN, LOW);
    last_high_millis = millis();
    if ((millis() - last_low_millis) <= 1.1 * DOT) {
      //Serial.println(".");
      letter_buffer[letter_buffer_index] = SYMBOL_DOT;
      letter_buffer_index += 1;
    } else if ((millis() - last_low_millis) <= 1.1 * DASH) {
      //Serial.println("-");
      letter_buffer[letter_buffer_index] = SYMBOL_DASH;
      letter_buffer_index += 1;
    }
    if (letter_buffer_index >= LETTER_BUFFER_SIZE) {
      // flush letter buffer
      for (int i = 0; i < LETTER_BUFFER_SIZE; ++i) letter_buffer[i] = 0;
      letter_buffer_index = 0;
    }
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    last_low_millis = millis();
    unsigned int time_since_high = millis() - last_high_millis;
    if (time_since_high > 0.9 * RECV_LETTER_SPACE) {
      parse_letter_buffer();
    }
    if (time_since_high > 0.9 * RECV_WORD_SPACE && word_buffer_index > 0) {
      word_complete = true;
    }
    if (time_since_high > ERASE_TIMEOUT) {
      flush_buffers();
    }
  }
}


void setup() {
  // Serial setup (optional, can be used for debugging output)
  Serial.begin(115200);

  // input is with pullup resistor, therefore "LOW" on that pin is signal
  pinMode(PIN_IN, INPUT_PULLUP);
  // use builtin LED to visualize what is sent
  pinMode(LED_BUILTIN, OUTPUT);
  // turn off LED initially
  digitalWrite(LED_BUILTIN, LOW);
}

int last_value = 0;

void loop() {
  // If the signal changes, call processing code. We do this on change as this code
  // would be simple to transfer to an interrupt later.
  if (digitalRead(PIN_IN) != last_value) {
    last_value = digitalRead(PIN_IN);
    read_morse();
  }

  // If we do not receive any HIGH signal for a WORD_SPACE the word has ended, but
  // since there is no signal change anymore, read_morse() would not be called.
  if (millis() - last_high_millis > RECV_WORD_SPACE &&
      (word_buffer_index > 0 || letter_buffer_index > 0)) {
    parse_letter_buffer();       
    word_complete = true;
  }

  // We have a complete word, print it to serial
  if (word_complete) {
    if (word_buffer_index > 0) {  // any data at all?
      Serial.print("Received word: ");
      Serial.println((const char*)word_buffer);
    }
    word_complete = false;
    flush_buffers();
  }

  delay(10);
}
