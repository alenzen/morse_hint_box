// Morse Code Hint Box
// Copyright (C) 2023 Tim Niemueller

// Required libraries: GxEPD2, TimerOne

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_7C.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <TimerOne.h>

// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"

#define DEG2RAD 0.0174532925

// Morse pins
#define PIN_IN 3
#define PIN_OUT SDA

#define MORSE_CODE_LENGTH 36
const char *morse_code[MORSE_CODE_LENGTH] = {
  // a-z
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--",
  "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
  // 0-9
  "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."
};

// We accept German and Elvish, no Klingon this time :-)
#define SECRET_WORD_1 "freund"
#define SECRET_WORD_2 "mellon"

#define TIME_UNIT    (250)
#define DOT          TIME_UNIT
#define DASH         (3*TIME_UNIT)
#define SYMBOL_SPACE TIME_UNIT

#define SEND_LETTER_SPACE  (3*TIME_UNIT - SYMBOL_SPACE)
#define RECV_LETTER_SPACE  (3*TIME_UNIT)
#define SEND_WORD_SPACE    (7 * TIME_UNIT - SEND_LETTER_SPACE)
#define RECV_WORD_SPACE    (7 * TIME_UNIT)
#define RECV_ERASE_TIMEOUT (20 * TIME_UNIT)

#define LETTER_BUFFER_SIZE  8
#define WORD_BUFFER_SIZE   10

#define SYMBOL_DOT  '.'
#define SYMBOL_DASH '-'

// buffer to read strings from progmem
// We need progmem to store strings to reduce SRAM usage.
char progmem_buffer[40];

// Variables for RECEIVING
volatile unsigned long last_low_millis = 0;
volatile unsigned long last_high_millis = 0;
volatile char word_buffer[WORD_BUFFER_SIZE];
volatile char letter_buffer[LETTER_BUFFER_SIZE];
volatile byte letter_buffer_index = 0;
volatile byte word_buffer_index = 0;
volatile bool word_complete = false;

typedef enum {
  INIT,
  START,
  INSTRUCTIONS,
  HINT
} State;
State last_state = INIT;
volatile State state = START;

// Parse buffer of symbols for a single letter
void parse_letter_buffer() {
  for (int i = 0; i < MORSE_CODE_LENGTH; ++i) {
    if (strcmp(morse_code[i], letter_buffer) == 0) {
      if (i < ('z' - 'a')) {
        word_buffer[word_buffer_index] = 'a' + i;
      } else {
        word_buffer[word_buffer_index] = i - ('z' - 'a' + 1) + '0';
      }
      word_buffer_index += 1;
      if (word_buffer_index >= WORD_BUFFER_SIZE) {
        word_complete = true;
      }
    }
  }
  for (int i = 0; i < LETTER_BUFFER_SIZE; ++i) letter_buffer[i] = 0;
  letter_buffer_index = 0;
}

// Remove all data from inflow buffers
void flush_buffers() {
  for (int i = 0; i < LETTER_BUFFER_SIZE; ++i) letter_buffer[i] = 0;
  for (int i = 0; i < WORD_BUFFER_SIZE; ++i) word_buffer[i] = 0;
  letter_buffer_index = 0;
  word_buffer_index = 0;
  word_complete = false;
}

// Interrupt function to read morse code based on signal changes.
void read_morse() {
  if (digitalRead(PIN_IN) == HIGH) {
    digitalWrite(LED_BUILTIN, LOW);
    last_high_millis = millis();
    if ((millis() - last_low_millis) <= 1.1 * DOT) {
      //Serial.println(SYMBOL_DOT);
      letter_buffer[letter_buffer_index] = SYMBOL_DOT;
      letter_buffer_index += 1;
    } else if ((millis() - last_low_millis) <= 1.1 * DASH) {
      //Serial.println(SYMBOL_DASH);
      letter_buffer[letter_buffer_index] = SYMBOL_DASH;
      letter_buffer_index += 1;
		//} else {
      //state = INSTRUCTIONS;
    }
    if (letter_buffer_index >= LETTER_BUFFER_SIZE) {
      // overflow, flush letter buffer
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
    if (time_since_high > RECV_ERASE_TIMEOUT) {
      flush_buffers();
    }
  }
}

void write_for(int time_ms) {
  digitalWrite(PIN_OUT, LOW);
  //digitalWrite(LED_BUILTIN, HIGH);
  delay(time_ms);
  digitalWrite(PIN_OUT, HIGH);
  //digitalWrite(LED_BUILTIN, LOW);
  delay(SYMBOL_SPACE);
}

void write_morse(char* message) {
  Serial.print("Sending: ");
  Serial.println(message);
  for (int i = 0; i < strlen(message); ++i) {
    int letter = tolower(message[i]);
    if (letter == ' ') {
      delay(SEND_WORD_SPACE);
    } else if (letter >= 'a' && letter <= 'z') {
      const char* morse_letter = morse_code[letter - 'a'];
      for (int j = 0; j < strlen(morse_letter); ++j) {
        write_for((morse_letter[j] == '.') ? DOT : DASH);
      }
    } else if (letter >= '0' && letter <= '9') {
      const char* morse_letter = morse_code[letter + 26 - '0'];
      for (int j = 0; j < strlen(morse_letter); ++j) {
        write_for((morse_letter[j] == '.') ? DOT : DASH);
      }
    }
    delay(SEND_LETTER_SPACE);
  }
}


const char screenStartTitle[] PROGMEM = "Helga und Al Hint Vault";
const char screenStartRiddle1[] PROGMEM = "Sprich Freund";
const char screenStartRiddle2[] PROGMEM = "und tritt ein";
const char screenStartInstructions[] PROGMEM = "Ich lausche blau und spreche weiss";

void screenStart()
{
  display.setRotation(1);
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeSansBold12pt7b);
    display.setTextColor(GxEPD_YELLOW);
    int16_t tbx, tby; uint16_t tbw, tbh;
    strcpy_P(progmem_buffer, screenStartTitle);
    display.getTextBounds(progmem_buffer, 0, 0, &tbx, &tby, &tbw, &tbh);
    // center the bounding box by transposition of the origin:
    uint16_t top_x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = tbh+2;//((display.height() - tbh) / 2) - tby;
    display.setCursor(top_x, y);
    display.print(progmem_buffer);

    uint16_t radius = 20;
    fillArc2(display.width() - 3.25*radius, tbh+1.75*radius, 0, 120, radius+3, radius+3, 3, GxEPD_YELLOW);
    fillArc2(display.width() - 2.25*radius, tbh+2.75*radius, 0, 120, radius+3, radius+3, 3, GxEPD_BLACK);
    fillArc2(display.width() - 3.25*radius, tbh+1.75*radius, 170, 6, radius+3, radius+3, 3, GxEPD_YELLOW);

    display.setTextColor(GxEPD_BLACK);
    strcpy_P(progmem_buffer, screenStartRiddle1);
    display.getTextBounds(progmem_buffer, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint8_t x = top_x;
    y = tbh+2*radius;
    display.setCursor(x, y);
    display.print(progmem_buffer);

    strcpy_P(progmem_buffer, screenStartRiddle2);
    display.getTextBounds(progmem_buffer, top_x, y, &tbx, &tby, &tbw, &tbh);
    x = top_x + 40;
    y = y + tbh + 10;
    display.setCursor(x, y);
    display.print(progmem_buffer);

    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    strcpy_P(progmem_buffer, screenStartInstructions);
    display.getTextBounds(progmem_buffer, 0, 0, &tbx, &tby, &tbw, &tbh);
    x = ((display.width() - tbw) / 2) - tbx;
    y = (display.height() - tbh - 2) - tby;
    display.setCursor(x, y);
    display.print(progmem_buffer);

  } while (display.nextPage());
}

const char screenHintTitle[] PROGMEM = "Geschafft!";
const char screenHintRiddle1[] PROGMEM = "Und nun auf";
const char screenHintRiddle2[] PROGMEM = "zum lieben...";
const char screenHintGroup1[] PROGMEM = "Tob -> Buck -> Tim -> ...";
const char screenHintDots[] PROGMEM = "...";
const char screenHintGroup2[] PROGMEM = "Appel";

void screenHint1()
{
  display.setRotation(1);
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeSansBold12pt7b);
    display.setTextColor(GxEPD_YELLOW);
    int16_t tbx, tby; uint16_t tbw, tbh;
    strcpy_P(progmem_buffer, screenHintTitle);
    display.getTextBounds(progmem_buffer, 0, 0, &tbx, &tby, &tbw, &tbh);
    // center the bounding box by transposition of the origin:
    uint16_t top_x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = tbh+2;//((display.height() - tbh) / 2) - tby;
    display.setCursor(top_x, y);
    display.print(progmem_buffer);

    uint16_t radius = 20;
    fillArc2(display.width() - 3*radius, tbh+2.75*radius, 0, 120, radius+3, radius+3, 3, GxEPD_YELLOW);
    fillArc2(display.width() - 2.75*radius, tbh+3*radius, 0, 120, 5, 5, 5, GxEPD_YELLOW);
    fillArc2(display.width() - 2.25*radius, tbh+2*radius, 270, 30, 20, 15, 2, GxEPD_BLACK);

    display.setTextColor(GxEPD_BLACK);
    strcpy_P(progmem_buffer, screenHintRiddle1);
    display.getTextBounds(progmem_buffer, 0, 0, &tbx, &tby, &tbw, &tbh);
    y = tbh+2*radius;
    display.setCursor(20, y);
    display.print(progmem_buffer);

    strcpy_P(progmem_buffer, screenHintRiddle2);
    display.getTextBounds(progmem_buffer, top_x, y, &tbx, &tby, &tbw, &tbh);
    y = y + tbh + 10;
    display.setCursor(20 + 40, y);
    display.print(progmem_buffer);

    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    strcpy_P(progmem_buffer, screenHintGroup1);
    display.getTextBounds(progmem_buffer, 0, 0, &tbx, &tby, &tbw, &tbh);
    int16_t tbx2, tby2; uint16_t tbw2, tbh2;
    strcpy_P(progmem_buffer, screenHintGroup2);
    display.getTextBounds(progmem_buffer, 0, 0, &tbx2, &tby2, &tbw2, &tbh2);
    uint16_t x = ((display.width() - tbw - tbw2) / 2) - tbx;
    y = (display.height() - tbh - 2) - tby - 3;
    display.setCursor(x, y);
    strcpy_P(progmem_buffer, screenHintGroup1);
    display.print(progmem_buffer);

  } while (display.nextPage());
}

void screenHint2()
{
  display.setRotation(1);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);

    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSans9pt7b);
    int16_t tbx, tby; uint16_t tbw, tbh;
    strcpy_P(progmem_buffer, screenHintGroup1);
    display.getTextBounds(progmem_buffer, 0, 0, &tbx, &tby, &tbw, &tbh);
    int16_t tbx2, tby2; uint16_t tbw2, tbh2;
    strcpy_P(progmem_buffer, screenHintGroup2);
    display.getTextBounds(progmem_buffer, 0, 0, &tbx2, &tby2, &tbw2, &tbh2);
    int16_t tbx3, tby3; uint16_t tbw3, tbh3;
    strcpy_P(progmem_buffer, screenHintDots);
    display.getTextBounds(progmem_buffer, 0, 0, &tbx3, &tby3, &tbw3, &tbh3);


    uint16_t x = ((display.width() - tbw - tbw2) / 2) + tbw - tbw3 - tbx;
    uint16_t y = (display.height() - tbh - 2) - tby - 3;

    display.setPartialWindow(x, y - tbh2, tbw2, tbh2);
    display.setCursor(x, y);
    strcpy_P(progmem_buffer, screenHintGroup2);
    display.print(progmem_buffer);

  } while (display.nextPage());
}

// #########################################################################
// Draw a circular or elliptical arc with a defined thickness
// #########################################################################

// Various places on the web, don't know the actual origin
// I took this from https://forum.arduino.cc/t/adafruit_gfx-fillarc/397741/7

// x,y == coords of centre of arc
// start_angle = 0 - 359
// seg_count = number of 3 degree segments to draw (120 => 360 degree arc)
// rx = x axis radius
// yx = y axis radius
// w  = width (thickness) of arc in pixels
// colour = 16 bit colour value
// Note if rx and ry are the same then an arc of a circle is drawn

int fillArc2(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
{

  byte seg = 3; // Segments are 3 degrees wide = 120 segments for 360 degrees
  byte inc = 3; // Draw segments every 3 degrees, increase to 6 for segmented ring

    // Calculate first pair of coordinates for segment start
    float sx = cos((start_angle - 90) * DEG2RAD);
    float sy = sin((start_angle - 90) * DEG2RAD);
    uint16_t x0 = sx * (rx - w) + x;
    uint16_t y0 = sy * (ry - w) + y;
    uint16_t x1 = sx * rx + x;
    uint16_t y1 = sy * ry + y;

  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * DEG2RAD);
    float sy2 = sin((i + seg - 90) * DEG2RAD);
    int x2 = sx2 * (rx - w) + x;
    int y2 = sy2 * (ry - w) + y;
    int x3 = sx2 * rx + x;
    int y3 = sy2 * ry + y;

    display.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
    display.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

    // Copy segment end to sgement start for next segment
    x0 = x2;
    y0 = y2;
    x1 = x3;
    y1 = y3;
  }
}

void process_read_buffer() {
  noInterrupts();
  if (millis() - last_high_millis > RECV_WORD_SPACE &&
      (word_buffer_index > 0 || letter_buffer_index > 0)) {
    parse_letter_buffer();
    word_complete = true;
  }
  if (word_complete) {
    if (word_buffer_index > 0) {
      Serial.print("Received word: ");
      Serial.println((const char*)word_buffer);
      if (strcmp(word_buffer, SECRET_WORD_1) == 0 ||
          strcmp(word_buffer, SECRET_WORD_2) == 0) {
        state = HINT;
      }
    }
    word_complete = false;
    flush_buffers();
  }
  interrupts();
}

void setup()
{
  Serial.begin(115200);
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse

  pinMode(PIN_IN, INPUT_PULLUP);
  pinMode(PIN_OUT, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

	// Setup interrupt for morse reading
  attachInterrupt(digitalPinToInterrupt(PIN_IN), read_morse, CHANGE);

	// Initial signal HIGH and turn LED off
  digitalWrite(PIN_OUT, HIGH);
  digitalWrite(LED_BUILTIN, LOW);

	// Initialize timer for processing read buffer
  Timer1.initialize(10000);
  Timer1.attachInterrupt(process_read_buffer);

  Serial.println("Start");
}

unsigned long last_write_millis = 0;
void loop() {
  if (state != last_state) {
    last_state = state;
    switch (state) {
      case START:
        Serial.println("Switching state to START");
        screenStart();
        break;

      case INSTRUCTIONS:
        Serial.println("Switching state to INSTRUCTIONS");
        //helloWorld();
        break;

      case HINT:
        Serial.println("Switching state to HINT");
        screenHint1();
        delay(15000);
        screenHint2();
        break;

    }
    display.hibernate();
  }
  if (millis() - last_write_millis > 2500) {
    write_morse("hallo");
    last_write_millis = millis();
  }
  delay(10);
};
