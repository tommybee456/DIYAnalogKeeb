#include "ADS1X15.h"
#include <Encoder.h>
#include "EEPROM.h"
#include "main_image.h"

#include "USB.h"
#include "USBHIDKeyboard.h"
#include "USBHIDConsumerControl.h"

#define BOUNCE_WITH_PROMPT_DETECTION

#include <Bounce2.h>  // https://github.com/thomasfredericks/Bounce2

// #if ARDUINO_USB_CDC_ON_BOOT
// #define HWSerial Serial0
// #define USBSerial Serial
// #else
// #define HWSerial Serial
// USBCDC USBSerial;
// #endif

#include <Adafruit_SSD1331.h>
#include "Adafruit_GFX.h"

#define LED_BLACK 0

#define LED_RED_VERYLOW (3 << 11)
#define LED_RED_LOW (7 << 11)
#define LED_RED_MEDIUM (15 << 11)
#define LED_RED_HIGH (31 << 11)

#define LED_GREEN_VERYLOW (1 << 5)
#define LED_GREEN_LOW (15 << 5)
#define LED_GREEN_MEDIUM (31 << 5)
#define LED_GREEN_HIGH (63 << 5)

#define LED_BLUE_VERYLOW 3
#define LED_BLUE_LOW 7
#define LED_BLUE_MEDIUM 15
#define LED_BLUE_HIGH 31

#define LED_ORANGE_VERYLOW (LED_RED_VERYLOW + LED_GREEN_VERYLOW)
#define LED_ORANGE_LOW (LED_RED_LOW + LED_GREEN_LOW)
#define LED_ORANGE_MEDIUM (LED_RED_MEDIUM + LED_GREEN_MEDIUM)
#define LED_ORANGE_HIGH (LED_RED_HIGH + LED_GREEN_HIGH)

#define LED_PURPLE_VERYLOW (LED_RED_VERYLOW + LED_BLUE_VERYLOW)
#define LED_PURPLE_LOW (LED_RED_LOW + LED_BLUE_LOW)
#define LED_PURPLE_MEDIUM (LED_RED_MEDIUM + LED_BLUE_MEDIUM)
#define LED_PURPLE_HIGH (LED_RED_HIGH + LED_BLUE_HIGH)

#define LED_CYAN_VERYLOW (LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_CYAN_LOW (LED_GREEN_LOW + LED_BLUE_LOW)
#define LED_CYAN_MEDIUM (LED_GREEN_MEDIUM + LED_BLUE_MEDIUM)
#define LED_CYAN_HIGH (LED_GREEN_HIGH + LED_BLUE_HIGH)

#define LED_WHITE_VERYLOW (LED_RED_VERYLOW + LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_WHITE_LOW (LED_RED_LOW + LED_GREEN_LOW + LED_BLUE_LOW)
#define LED_WHITE_MEDIUM (LED_RED_MEDIUM + LED_GREEN_MEDIUM + LED_BLUE_MEDIUM)
#define LED_WHITE_HIGH (LED_RED_HIGH + LED_GREEN_HIGH + LED_BLUE_HIGH)

#define show endWrite
#define clear() fillScreen(0)

// You can use any (4 or) 5 pins
#define SCK 34   // marked SCL or CK on OLED board
#define MOSI 36  // marked SDA or SI on OLED board
#define MISO 35
#define SS 33   // marked CS or OC on OLED board
#define rst 21  // marked RES or R on OLED board
#define dc 18   // marked DC or sometimes (confusingly) RS on OLED board

#pragma message "Using SWSPI"
Adafruit_SSD1331 display = Adafruit_SSD1331(SS, dc, MOSI, SCK, rst);

// #pragma message "Using HWSPI"
// Adafruit_SSD1331 display = Adafruit_SSD1331(&SPI, SS, dc, rst);

// Color definitions
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF


const unsigned long PADDLE_RATE = 35;
unsigned long BALL_RATE = 25;
const uint8_t PADDLE_HEIGHT = 22;
int MAX_SCORE = 8;

int CPU_SCORE = 0;
int PLAYER_SCORE = 0;

void drawCourt();

int ball_x = 64, ball_y = 32;
int ball_dir_x = 1, ball_dir_y = 1;

boolean gameIsRunning = true;
boolean resetBall = false;

static const unsigned char pong[] PROGMEM = {
  0xff, 0xe0, 0x0, 0x3f, 0x80, 0x7, 0xe0, 0x7, 0xc0, 0x3, 0xfc, 0x0,
  0xff, 0xf8, 0x1, 0xff, 0xe0, 0x7, 0xf0, 0x7, 0xc0, 0x1f, 0xff, 0x0,
  0xff, 0xfc, 0x3, 0xff, 0xf0, 0x7, 0xf0, 0x7, 0xc0, 0x3f, 0xff, 0x0,
  0xff, 0xfe, 0x7, 0xff, 0xf8, 0x7, 0xf8, 0x7, 0xc0, 0xff, 0xff, 0x0,
  0xf8, 0x7f, 0xf, 0xff, 0xfc, 0x7, 0xfc, 0x7, 0xc0, 0xff, 0xff, 0x0,
  0xf8, 0x3f, 0xf, 0xe0, 0xfe, 0x7, 0xfc, 0x7, 0xc1, 0xfc, 0x7, 0x0,
  0xf8, 0x1f, 0x1f, 0x80, 0x7e, 0x7, 0xfe, 0x7, 0xc3, 0xf8, 0x1, 0x0,
  0xf8, 0x1f, 0x1f, 0x0, 0x3e, 0x7, 0xfe, 0x7, 0xc3, 0xf0, 0x0, 0x0,
  0xf8, 0x1f, 0x3f, 0x0, 0x3f, 0x7, 0xdf, 0x7, 0xc7, 0xe0, 0x0, 0x0,
  0xf8, 0x1f, 0x3e, 0x0, 0x1f, 0x7, 0xdf, 0x87, 0xc7, 0xc0, 0x0, 0x0,
  0xf8, 0x3f, 0x3e, 0x0, 0x1f, 0x7, 0xcf, 0x87, 0xc7, 0xc1, 0xff, 0x80,
  0xf8, 0x7e, 0x3e, 0x0, 0x1f, 0x7, 0xc7, 0xc7, 0xc7, 0xc1, 0xff, 0x80,
  0xff, 0xfe, 0x3e, 0x0, 0x1f, 0x7, 0xc7, 0xe7, 0xc7, 0xc1, 0xff, 0x80,
  0xff, 0xfc, 0x3e, 0x0, 0x1f, 0x7, 0xc3, 0xe7, 0xc7, 0xc1, 0xff, 0x80,
  0xff, 0xf8, 0x3e, 0x0, 0x1f, 0x7, 0xc1, 0xf7, 0xc7, 0xc0, 0xf, 0x80,
  0xff, 0xe0, 0x3f, 0x0, 0x3f, 0x7, 0xc1, 0xf7, 0xc7, 0xe0, 0xf, 0x80,
  0xf8, 0x0, 0x1f, 0x0, 0x3e, 0x7, 0xc0, 0xff, 0xc3, 0xe0, 0xf, 0x80,
  0xf8, 0x0, 0x1f, 0x80, 0x7e, 0x7, 0xc0, 0x7f, 0xc3, 0xf0, 0xf, 0x80,
  0xf8, 0x0, 0x1f, 0xc0, 0xfc, 0x7, 0xc0, 0x7f, 0xc3, 0xfc, 0xf, 0x80,
  0xf8, 0x0, 0xf, 0xff, 0xfc, 0x7, 0xc0, 0x3f, 0xc1, 0xff, 0xff, 0x80,
  0xf8, 0x0, 0x7, 0xff, 0xf8, 0x7, 0xc0, 0x3f, 0xc0, 0xff, 0xff, 0x80,
  0xf8, 0x0, 0x3, 0xff, 0xf0, 0x7, 0xc0, 0x1f, 0xc0, 0x7f, 0xff, 0x80,
  0xf8, 0x0, 0x1, 0xff, 0xe0, 0x7, 0xc0, 0xf, 0xc0, 0x3f, 0xff, 0x0,
  0xf8, 0x0, 0x0, 0x7f, 0x0, 0x7, 0xc0, 0xf, 0xc0, 0x7, 0xf8, 0x0
};

static const unsigned char game[] PROGMEM = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x80,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0xff, 0x80,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xc3, 0xff, 0x80,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf, 0xc3, 0xff, 0x80,
  0x0, 0x0, 0x0, 0x0, 0x3, 0xe0, 0xf, 0xc3, 0xe0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x7, 0xf0, 0x1f, 0xc1, 0xe0, 0x0,
  0x0, 0x0, 0x0, 0xf8, 0x7, 0xf0, 0x1f, 0xc1, 0xe0, 0x0,
  0x0, 0xfc, 0x1, 0xfc, 0x7, 0xf8, 0x1f, 0xc1, 0xe0, 0x0,
  0x7, 0xfc, 0x1, 0xfc, 0x3, 0xf8, 0x1f, 0xe1, 0xff, 0x80,
  0x1f, 0xfc, 0x1, 0xde, 0x3, 0xbc, 0x3d, 0xe1, 0xff, 0x80,
  0x3f, 0xfe, 0x1, 0xde, 0x3, 0xbc, 0x39, 0xe1, 0xff, 0x80,
  0x7e, 0x0, 0x3, 0xdf, 0x3, 0xde, 0x39, 0xe1, 0xfc, 0x0,
  0x7c, 0x0, 0x3, 0xcf, 0x3, 0xde, 0x39, 0xe1, 0xe0, 0x0,
  0xf8, 0x0, 0x3, 0xcf, 0x3, 0xcf, 0x39, 0xe1, 0xf0, 0x0,
  0xf8, 0x0, 0x3, 0x87, 0x83, 0xcf, 0x79, 0xe0, 0xf0, 0x0,
  0xf0, 0x7f, 0x7, 0x87, 0x83, 0xc7, 0xf1, 0xe0, 0xf0, 0xe0,
  0xf0, 0xff, 0x7, 0x83, 0xc3, 0xc7, 0xf1, 0xe0, 0xff, 0xe0,
  0xf0, 0xff, 0x7, 0xff, 0xc1, 0xc3, 0xf1, 0xf0, 0xff, 0xe0,
  0xf0, 0xff, 0x7, 0xff, 0xe1, 0xc3, 0xf0, 0xf0, 0xff, 0xe0,
  0xf8, 0xf, 0xf, 0xff, 0xe1, 0xc1, 0xe0, 0xf0, 0xe0, 0x0,
  0xf8, 0xf, 0x8f, 0x1, 0xf1, 0xe1, 0xe0, 0xf0, 0x0, 0x0,
  0x7c, 0xf, 0x8f, 0x0, 0xf1, 0xe1, 0xe0, 0x0, 0x0, 0x0,
  0x7f, 0x1f, 0x8f, 0x0, 0xf9, 0xc0, 0x0, 0x0, 0x0, 0x0,
  0x3f, 0xff, 0x9f, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x1f, 0xff, 0x1f, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x7, 0xfc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

unsigned long ball_update;
unsigned long paddle_update;

const uint8_t CPU_X = 10;
int cpu_y = 16;

const uint8_t PLAYER_X = 87;
int player_y = 16;

USBHIDKeyboard Keyboard;
USBHIDConsumerControl ConsumerControl;

ADS1115 ADS0(0x48);
ADS1115 ADS1(0x49);
ADS1115 ADS2(0x4A);

ADS1115 ADS3(0x48, &Wire1);
ADS1115 ADS4(0x49, &Wire1);
ADS1115 ADS5(0x4A, &Wire1);

int16_t raw[24];
int16_t lim_low[24];
int16_t lim_high[24];
int16_t keys[24];
int16_t keys_old[24];
bool keysPressed[24] = { 0 };
bool keysPressed_old[24] = { 0 };
bool SOCD = 0;
byte actuation = 0;

byte wallpaper_num = 1;
byte SOCD_location = 1;
byte actuation_location = 1;
byte rp_sens = 2;
byte actuation_point = 5;

byte SOCD_num_saved = 0;
byte wallpaper_num_saved = 1;

byte actuation_saved;
byte rp_sens_saved;
byte actuation_point_saved;

uint8_t key_names[24] = {
  0x35,            //5
  0x34,            //4
  0x33,            //3
  0x32,            //2
  0x31,            //1
  0x60,            //`
  KEY_TAB,         //tab
  0x71,            //q
  0x77,            //w
  0x65,            //e
  0x72,            //r
  0x74,            //t
  0x67,            //g
  0x66,            //f
  0x64,            //d
  0x73,            //s
  0x61,            //a
  KEY_LEFT_SHIFT,  //shift
  0x78,            //x
  0x63,            //c
  0x76,            //v
  0x62,            //b
  0x20,            //space
  KEY_LEFT_CTRL    //cntl
};

int addr = 0;
#define EEPROM_SIZE 101

unsigned long ADC_timer = 0;
unsigned long ADC_timer_old = 0;
int count = 0;

unsigned long calib_timer;
unsigned long calib_timer_old;

unsigned long image_timer;
unsigned long image_timer_old;
int image_count = 0;

float high_deadzone = 0.98;
float low_deadzone = 1.01;

int SW = 4;
Encoder myEnc(6, 7);

long oldPosition;
long newPosition;

Bounce escape;

void readData() {
  lim_high[0] = EEPROM.read(0) << 8 | EEPROM.read(1);
  lim_high[1] = EEPROM.read(2) << 8 | EEPROM.read(3);
  lim_high[2] = EEPROM.read(4) << 8 | EEPROM.read(5);
  lim_high[3] = EEPROM.read(6) << 8 | EEPROM.read(7);
  lim_high[4] = EEPROM.read(8) << 8 | EEPROM.read(9);
  lim_high[5] = EEPROM.read(10) << 8 | EEPROM.read(11);
  lim_high[6] = EEPROM.read(12) << 8 | EEPROM.read(13);
  lim_high[7] = EEPROM.read(14) << 8 | EEPROM.read(15);
  lim_high[8] = EEPROM.read(16) << 8 | EEPROM.read(17);
  lim_high[9] = EEPROM.read(18) << 8 | EEPROM.read(19);
  lim_high[10] = EEPROM.read(20) << 8 | EEPROM.read(21);
  lim_high[11] = EEPROM.read(22) << 8 | EEPROM.read(23);
  lim_high[12] = EEPROM.read(24) << 8 | EEPROM.read(25);
  lim_high[13] = EEPROM.read(26) << 8 | EEPROM.read(27);
  lim_high[14] = EEPROM.read(28) << 8 | EEPROM.read(29);
  lim_high[15] = EEPROM.read(30) << 8 | EEPROM.read(31);
  lim_high[16] = EEPROM.read(32) << 8 | EEPROM.read(33);
  lim_high[17] = EEPROM.read(34) << 8 | EEPROM.read(35);
  lim_high[18] = EEPROM.read(36) << 8 | EEPROM.read(37);
  lim_high[19] = EEPROM.read(38) << 8 | EEPROM.read(39);
  lim_high[20] = EEPROM.read(40) << 8 | EEPROM.read(41);
  lim_high[21] = EEPROM.read(42) << 8 | EEPROM.read(43);
  lim_high[22] = EEPROM.read(44) << 8 | EEPROM.read(45);
  lim_high[23] = EEPROM.read(46) << 8 | EEPROM.read(47);

  lim_low[0] = EEPROM.read(48) << 8 | EEPROM.read(49);
  lim_low[1] = EEPROM.read(50) << 8 | EEPROM.read(51);
  lim_low[2] = EEPROM.read(52) << 8 | EEPROM.read(53);
  lim_low[3] = EEPROM.read(54) << 8 | EEPROM.read(55);
  lim_low[4] = EEPROM.read(56) << 8 | EEPROM.read(57);
  lim_low[5] = EEPROM.read(58) << 8 | EEPROM.read(59);
  lim_low[6] = EEPROM.read(60) << 8 | EEPROM.read(61);
  lim_low[7] = EEPROM.read(62) << 8 | EEPROM.read(63);
  lim_low[8] = EEPROM.read(64) << 8 | EEPROM.read(65);
  lim_low[9] = EEPROM.read(66) << 8 | EEPROM.read(67);
  lim_low[10] = EEPROM.read(68) << 8 | EEPROM.read(69);
  lim_low[11] = EEPROM.read(70) << 8 | EEPROM.read(71);
  lim_low[12] = EEPROM.read(72) << 8 | EEPROM.read(73);
  lim_low[13] = EEPROM.read(74) << 8 | EEPROM.read(75);
  lim_low[14] = EEPROM.read(76) << 8 | EEPROM.read(77);
  lim_low[15] = EEPROM.read(78) << 8 | EEPROM.read(79);
  lim_low[16] = EEPROM.read(80) << 8 | EEPROM.read(81);
  lim_low[17] = EEPROM.read(82) << 8 | EEPROM.read(83);
  lim_low[18] = EEPROM.read(84) << 8 | EEPROM.read(85);
  lim_low[19] = EEPROM.read(86) << 8 | EEPROM.read(87);
  lim_low[20] = EEPROM.read(88) << 8 | EEPROM.read(89);
  lim_low[21] = EEPROM.read(90) << 8 | EEPROM.read(91);
  lim_low[22] = EEPROM.read(92) << 8 | EEPROM.read(93);
  lim_low[23] = EEPROM.read(94) << 8 | EEPROM.read(95);
}

void setup() {
  // pinMode(SCK, OUTPUT);
  // pinMode(MOSI, OUTPUT);
  // pinMode(MISO, INPUT);
  // pinMode(SS, OUTPUT);
  // pinMode(dc, OUTPUT);

  // digitalWrite(SS, HIGH);

  // SPI.begin(SCK, MISO, MOSI, SS);
  // SPI.setClockDivider(4);
  // SPI.setBitOrder(MSBFIRST);
  // SPI.setDataMode(SPI_MODE0);

  display.begin(40000000);
  display.clear();

  // display.drawRGBBitmap(0, 0, (const uint16_t *)main_image, 96, 64);
  // display.show();
  // Serial.begin(115200);

  pinMode(SW, INPUT_PULLUP);

  escape = Bounce();
  escape.attach(SW);  // After setting up the button, setup the Bounce instance :
  escape.interval(50);

  if (!EEPROM.begin(EEPROM_SIZE)) {
    // Serial.println("failed to initialise EEPROM");
    delay(1000000);
  }

  if (EEPROM.read(0) == 255 && EEPROM.read(1) == 255) {
    for (int i = 0; i < 24; i++) {
      lim_low[i] = 0;
      lim_high[i] = 15000;
    }
  } else {
    readData();
  }

  if (EEPROM.read(96) != 255) {
    SOCD_num_saved = EEPROM.read(96);
    SOCD = SOCD_num_saved;
  }

  if (EEPROM.read(97) != 255) {
    wallpaper_num_saved = EEPROM.read(97);
    wallpaper_num = wallpaper_num_saved;
    if (wallpaper_num == 1) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)main_image, 96, 64);
    }
    if (wallpaper_num == 2) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)skull, 96, 64);
    }
    if (wallpaper_num == 3) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)rainbow, 96, 64);
    }
    if (wallpaper_num == 4) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)cats, 96, 64);
    }
    if (wallpaper_num == 5) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)tommyB, 96, 64);
    }
    if (wallpaper_num == 6) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)colorBars, 96, 64);
    }
    if (wallpaper_num == 7) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)ak47, 96, 64);
    }
    if (wallpaper_num == 8) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)geometric, 96, 64);
    }
  } else {
    display.drawRGBBitmap(0, 0, (const uint16_t *)main_image, 96, 64);
  }
  display.show();
  if (EEPROM.read(98) != 255) {
    actuation = EEPROM.read(98);
    actuation_saved = actuation;
  }
  if (EEPROM.read(99) != 255) {
    rp_sens = EEPROM.read(99);
    rp_sens_saved = rp_sens;
  }
  if (EEPROM.read(100) != 255) {
    actuation_point = EEPROM.read(100);
    actuation_point_saved = actuation_point;
  }

  Wire.begin(2, 3);
  Wire.setClock(3400000);
  Wire1.begin(40, 39);
  Wire1.setClock(3400000);

  ADS0.begin();
  ADS0.setDataRate(7);
  ADS0.setGain(1);
  ADS0.setMode(1);

  ADS0.requestADC(0);



  ADS1.begin();
  ADS1.setDataRate(7);
  ADS1.setGain(1);
  ADS1.setMode(1);

  ADS1.requestADC(0);


  ADS2.begin();
  ADS2.setDataRate(7);
  ADS2.setGain(1);
  ADS2.setMode(1);

  ADS2.requestADC(0);

  ADS3.begin();
  ADS3.setDataRate(7);
  ADS3.setGain(1);
  ADS3.setMode(1);

  ADS3.requestADC(0);



  ADS4.begin();
  ADS4.setDataRate(7);
  ADS4.setGain(1);
  ADS4.setMode(1);

  ADS4.requestADC(0);


  ADS5.begin();
  ADS5.setDataRate(7);
  ADS5.setGain(1);
  ADS5.setMode(1);

  ADS5.requestADC(0);


  for (int i = 0; i < 12; i++) {
    readSwitches();
    keys_old[i] = keys[i];
  }
  filterSwitches();
  for (int i = 0; i < 24; i++) {
    keys_old[i] = keys[i];
  }

  Keyboard.begin();  //all this calls is hid.begin() so I dont need to also do ConsumerControl.begin() here since it would be redundant
  USB.begin();

  newPosition = myEnc.read();
  oldPosition = newPosition;

  delay(1000);
  // image_timer = millis();
  // image_timer_old = image_timer;
}

void loop() {
  readSwitches();
  filterSwitches();
  volume_control();  //does not work with basic library, will have to edit to get this to work
  escape.update();
  // int lag = micros();
  if (escape.fell()) {
    Keyboard.press(0xB1);
  }
  if (escape.rose()) {
    Keyboard.release(0xB1);
  }
  if (digitalRead(SW) == 0) {
    calib_timer = millis();
    if (calib_timer - calib_timer_old >= 3000) {
      menu_control();
      calib_timer_old = calib_timer;
    }

  } else {
    calib_timer_old = millis();
  }
  for (int i = 0; i < 24; i++) {
    if (!actuation) {
      if (SOCD && (i == 8 || i == 14 || i == 15 || i == 16)) {
        //do nothing
      } else {
        if (keys[i] - keys_old[i] >= rp_sens) {
          keysPressed[i] = 0;
          keys_old[i] = keys[i];
          Keyboard.release(key_names[i]);
        }
        if (keys_old[i] - keys[i] >= rp_sens) {
          keysPressed[i] = 1;
          keys_old[i] = keys[i];
          Keyboard.press(key_names[i]);
        }
      }
    } else {
      if (SOCD && (i == 8 || i == 14 || i == 15 || i == 16)) {
        //do nothing
      } else {
        if (keys[i] >= actuation_point) {
          keysPressed[i] = 0;
        }
        if (keys[i] < actuation_point) {
          keysPressed[i] = 1;
        }
        if (keysPressed[i] != keysPressed_old[i]) {
          if (keysPressed[i] == 0) {
            keysPressed_old[i] = keysPressed[i];
            Keyboard.release(key_names[i]);
          }
          if (keysPressed[i] == 1) {
            keysPressed_old[i] = keysPressed[i];
            Keyboard.press(key_names[i]);
          }
        }
      }
    }
  }
  if (SOCD) {
    socd_on();
  }

  // Serial.print(SOCD);
  // Serial.print('\t');
  // Serial.print(SOCD_num_saved);
  // Serial.print('\t');
  // Serial.print(wallpaper_num_saved);
  // Serial.print('\t');
  // Serial.println(wallpaper_num);
  // Serial.println(micros() - lag);
}

void readSwitches() {
  ADC_timer = millis();
  if (ADC_timer - ADC_timer_old >= 1) {
    if (ADS0.isReady() && count == 0) {
      raw[0] = ADS0.readADC(0);
      raw[1] = ADS0.readADC(1);
      raw[2] = ADS0.readADC(2);
      raw[3] = ADS0.readADC(3);
      count = 1;
    } else if (ADS3.isReady() && count == 1) {
      raw[12] = ADS3.readADC(0);
      raw[13] = ADS3.readADC(1);
      raw[14] = ADS3.readADC(2);
      raw[15] = ADS3.readADC(3);
      count = 2;
    } else if (ADS2.isReady() && count == 2) {
      raw[8] = ADS2.readADC(0);
      raw[9] = ADS2.readADC(1);
      raw[10] = ADS2.readADC(2);
      raw[11] = ADS2.readADC(3);
      count = 3;
      ADC_timer_old = ADC_timer;
    } else if (ADS4.isReady() && count == 3) {
      raw[16] = ADS4.readADC(0);
      raw[17] = ADS4.readADC(1);
      raw[18] = ADS4.readADC(2);
      raw[19] = ADS4.readADC(3);
      count = 4;
    } else if (ADS1.isReady() && count == 4) {
      raw[4] = ADS1.readADC(0);
      raw[5] = ADS1.readADC(1);
      raw[6] = ADS1.readADC(2);
      raw[7] = ADS1.readADC(3);
      count = 5;
    } else if (ADS5.isReady() && count == 5) {
      raw[20] = ADS5.readADC(0);
      raw[21] = ADS5.readADC(1);
      raw[22] = ADS5.readADC(2);
      raw[23] = ADS5.readADC(3);
      count = 0;
      ADC_timer_old = ADC_timer;
    }
  }
}

long fastMap(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void filterSwitches() {
  for (int i = 0; i < 24; i++) {
    keys[i] = fastMap(raw[i], lim_low[i], lim_high[i], 0, 10);
  }
}

void calibrate() {
  display.clear();
  display.setCursor(0, 10);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("  Calibration:");
  display.setCursor(0, 25);
  display.print("Please let go of");
  display.setCursor(0, 35);
  display.print("all keys and");
  display.setCursor(0, 45);
  display.print("press the knob");
  display.show();
  while (true) {
    escape.update();
    if (escape.fell()) {
      break;
    }
    for (int i = 0; i < 12; i++) {
      readSwitches();
    }
    for (int i = 0; i < 24; i++) {
      lim_high[i] = raw[i];
    }
  }
  display.clear();
  display.setCursor(0, 10);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("  Calibration:");
  display.setCursor(0, 25);
  display.print("Please push the");
  display.setCursor(0, 35);
  display.print("top row of keys");
  display.setCursor(0, 45);
  display.print("and press the");
  display.setCursor(0, 55);
  display.print("knob");
  display.show();
  while (true) {
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  for (int i = 0; i < 12; i++) {
    readSwitches();
  }
  for (int i = 0; i < 6; i++) {
    lim_low[i] = raw[i];
  }
  display.clear();
  display.setCursor(0, 10);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("  Calibration:");
  display.setCursor(0, 25);
  display.print("Please push the");
  display.setCursor(0, 35);
  display.print("2nd row of keys");
  display.setCursor(0, 45);
  display.print("and press the");
  display.setCursor(0, 55);
  display.print("knob");
  display.show();
  while (true) {
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  for (int i = 0; i < 12; i++) {
    readSwitches();
  }
  for (int i = 6; i < 12; i++) {
    lim_low[i] = raw[i];
  }
  display.clear();
  display.setCursor(0, 10);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("  Calibration:");
  display.setCursor(0, 25);
  display.print("Please push the");
  display.setCursor(0, 35);
  display.print("3rd row of keys");
  display.setCursor(0, 45);
  display.print("and press the");
  display.setCursor(0, 55);
  display.print("knob");
  display.show();
  while (true) {
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  for (int i = 0; i < 12; i++) {
    readSwitches();
  }
  for (int i = 12; i < 17; i++) {
    lim_low[i] = raw[i];
  }
  display.clear();
  display.setCursor(0, 10);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("  Calibration:");
  display.setCursor(0, 25);
  display.print("Please push the");
  display.setCursor(0, 35);
  display.print("4th row of keys");
  display.setCursor(0, 45);
  display.print("and press the");
  display.setCursor(0, 55);
  display.print("knob");
  display.show();
  while (true) {
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  for (int i = 0; i < 12; i++) {
    readSwitches();
  }
  for (int i = 17; i < 22; i++) {
    lim_low[i] = raw[i];
  }

  display.clear();
  display.setCursor(0, 10);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("  Calibration:");
  display.setCursor(0, 25);
  display.print("Please push the");
  display.setCursor(0, 35);
  display.print("5th row of keys");
  display.setCursor(0, 45);
  display.print("and press the");
  display.setCursor(0, 55);
  display.print("knob");
  display.show();
  while (true) {
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  for (int i = 0; i < 12; i++) {
    readSwitches();
  }
  for (int i = 22; i < 24; i++) {
    lim_low[i] = raw[i];
  }
  display.clear();
  display.setCursor(0, 10);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("Calibration Done");
  display.setCursor(10, 30);
  display.print("Save and Quit");
  display.setCursor(10, 45);
  display.print("Just Quit");
  display.drawCircle(4, 32, 2, LED_WHITE_HIGH);
  display.show();
  bool to_save = 1;
  while (true) {
    newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      if (newPosition < oldPosition - 1) {
        oldPosition = newPosition;
        display.drawCircle(4, 32, 2, LED_BLACK);
        display.drawCircle(4, 47, 2, LED_WHITE_HIGH);
        display.show();
        to_save = 0;
      }
      if (newPosition > oldPosition + 1) {
        oldPosition = newPosition;
        display.drawCircle(4, 32, 2, LED_WHITE_HIGH);
        display.drawCircle(4, 47, 2, LED_BLACK);
        display.show();
        to_save = 1;
      }
    }
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  for (int i = 0; i < 24; i++) {
    lim_low[i] = lim_low[i] * low_deadzone;
    lim_high[i] = lim_high[i] * high_deadzone;
  }
  if (to_save) {
    save_eeprom();
  }
  ESP.restart();
}

void save_eeprom() {
  uint8_t eeprom_data[96];
  eeprom_data[0] = (lim_high[0] >> 8) & 0xff;
  eeprom_data[1] = (lim_high[0] >> 0) & 0xff;
  eeprom_data[2] = (lim_high[1] >> 8) & 0xff;
  eeprom_data[3] = (lim_high[1] >> 0) & 0xff;
  eeprom_data[4] = (lim_high[2] >> 8) & 0xff;
  eeprom_data[5] = (lim_high[2] >> 0) & 0xff;
  eeprom_data[6] = (lim_high[3] >> 8) & 0xff;
  eeprom_data[7] = (lim_high[3] >> 0) & 0xff;
  eeprom_data[8] = (lim_high[4] >> 8) & 0xff;
  eeprom_data[9] = (lim_high[4] >> 0) & 0xff;
  eeprom_data[10] = (lim_high[5] >> 8) & 0xff;
  eeprom_data[11] = (lim_high[5] >> 0) & 0xff;
  eeprom_data[12] = (lim_high[6] >> 8) & 0xff;
  eeprom_data[13] = (lim_high[6] >> 0) & 0xff;
  eeprom_data[14] = (lim_high[7] >> 8) & 0xff;
  eeprom_data[15] = (lim_high[7] >> 0) & 0xff;
  eeprom_data[16] = (lim_high[8] >> 8) & 0xff;
  eeprom_data[17] = (lim_high[8] >> 0) & 0xff;
  eeprom_data[18] = (lim_high[9] >> 8) & 0xff;
  eeprom_data[19] = (lim_high[9] >> 0) & 0xff;
  eeprom_data[20] = (lim_high[10] >> 8) & 0xff;
  eeprom_data[21] = (lim_high[10] >> 0) & 0xff;
  eeprom_data[22] = (lim_high[11] >> 8) & 0xff;
  eeprom_data[23] = (lim_high[11] >> 0) & 0xff;
  eeprom_data[24] = (lim_high[12] >> 8) & 0xff;
  eeprom_data[25] = (lim_high[12] >> 0) & 0xff;
  eeprom_data[26] = (lim_high[13] >> 8) & 0xff;
  eeprom_data[27] = (lim_high[13] >> 0) & 0xff;
  eeprom_data[28] = (lim_high[14] >> 8) & 0xff;
  eeprom_data[29] = (lim_high[14] >> 0) & 0xff;
  eeprom_data[30] = (lim_high[15] >> 8) & 0xff;
  eeprom_data[31] = (lim_high[15] >> 0) & 0xff;
  eeprom_data[32] = (lim_high[16] >> 8) & 0xff;
  eeprom_data[33] = (lim_high[16] >> 0) & 0xff;
  eeprom_data[34] = (lim_high[17] >> 8) & 0xff;
  eeprom_data[35] = (lim_high[17] >> 0) & 0xff;
  eeprom_data[36] = (lim_high[18] >> 8) & 0xff;
  eeprom_data[37] = (lim_high[18] >> 0) & 0xff;
  eeprom_data[38] = (lim_high[19] >> 8) & 0xff;
  eeprom_data[39] = (lim_high[19] >> 0) & 0xff;
  eeprom_data[40] = (lim_high[20] >> 8) & 0xff;
  eeprom_data[41] = (lim_high[20] >> 0) & 0xff;
  eeprom_data[42] = (lim_high[21] >> 8) & 0xff;
  eeprom_data[43] = (lim_high[21] >> 0) & 0xff;
  eeprom_data[44] = (lim_high[22] >> 8) & 0xff;
  eeprom_data[45] = (lim_high[22] >> 0) & 0xff;
  eeprom_data[46] = (lim_high[23] >> 8) & 0xff;
  eeprom_data[47] = (lim_high[23] >> 0) & 0xff;

  eeprom_data[48] = (lim_low[0] >> 8) & 0xff;
  eeprom_data[49] = (lim_low[0] >> 0) & 0xff;
  eeprom_data[50] = (lim_low[1] >> 8) & 0xff;
  eeprom_data[51] = (lim_low[1] >> 0) & 0xff;
  eeprom_data[52] = (lim_low[2] >> 8) & 0xff;
  eeprom_data[53] = (lim_low[2] >> 0) & 0xff;
  eeprom_data[54] = (lim_low[3] >> 8) & 0xff;
  eeprom_data[55] = (lim_low[3] >> 0) & 0xff;
  eeprom_data[56] = (lim_low[4] >> 8) & 0xff;
  eeprom_data[57] = (lim_low[4] >> 0) & 0xff;
  eeprom_data[58] = (lim_low[5] >> 8) & 0xff;
  eeprom_data[59] = (lim_low[5] >> 0) & 0xff;
  eeprom_data[60] = (lim_low[6] >> 8) & 0xff;
  eeprom_data[61] = (lim_low[6] >> 0) & 0xff;
  eeprom_data[62] = (lim_low[7] >> 8) & 0xff;
  eeprom_data[63] = (lim_low[7] >> 0) & 0xff;
  eeprom_data[64] = (lim_low[8] >> 8) & 0xff;
  eeprom_data[65] = (lim_low[8] >> 0) & 0xff;
  eeprom_data[66] = (lim_low[9] >> 8) & 0xff;
  eeprom_data[67] = (lim_low[9] >> 0) & 0xff;
  eeprom_data[68] = (lim_low[10] >> 8) & 0xff;
  eeprom_data[69] = (lim_low[10] >> 0) & 0xff;
  eeprom_data[70] = (lim_low[11] >> 8) & 0xff;
  eeprom_data[71] = (lim_low[11] >> 0) & 0xff;
  eeprom_data[72] = (lim_low[12] >> 8) & 0xff;
  eeprom_data[73] = (lim_low[12] >> 0) & 0xff;
  eeprom_data[74] = (lim_low[13] >> 8) & 0xff;
  eeprom_data[75] = (lim_low[13] >> 0) & 0xff;
  eeprom_data[76] = (lim_low[14] >> 8) & 0xff;
  eeprom_data[77] = (lim_low[14] >> 0) & 0xff;
  eeprom_data[78] = (lim_low[15] >> 8) & 0xff;
  eeprom_data[79] = (lim_low[15] >> 0) & 0xff;
  eeprom_data[80] = (lim_low[16] >> 8) & 0xff;
  eeprom_data[81] = (lim_low[16] >> 0) & 0xff;
  eeprom_data[82] = (lim_low[17] >> 8) & 0xff;
  eeprom_data[83] = (lim_low[17] >> 0) & 0xff;
  eeprom_data[84] = (lim_low[18] >> 8) & 0xff;
  eeprom_data[85] = (lim_low[18] >> 0) & 0xff;
  eeprom_data[86] = (lim_low[19] >> 8) & 0xff;
  eeprom_data[87] = (lim_low[19] >> 0) & 0xff;
  eeprom_data[88] = (lim_low[20] >> 8) & 0xff;
  eeprom_data[89] = (lim_low[20] >> 0) & 0xff;
  eeprom_data[90] = (lim_low[21] >> 8) & 0xff;
  eeprom_data[91] = (lim_low[21] >> 0) & 0xff;
  eeprom_data[92] = (lim_low[22] >> 8) & 0xff;
  eeprom_data[93] = (lim_low[22] >> 0) & 0xff;
  eeprom_data[94] = (lim_low[23] >> 8) & 0xff;
  eeprom_data[95] = (lim_low[23] >> 0) & 0xff;

  for (int i = 0; i < 96; i++) {
    EEPROM.write(i, eeprom_data[i]);
  }
  EEPROM.commit();
}

void socd_on() {
  if (!actuation) {
    if (keys[8] - keys_old[8] >= rp_sens) {
      keysPressed[8] = 0;
    }
    if (keys_old[8] - keys[8] >= rp_sens) {  //w is presssed
      keysPressed[8] = 1;
    }
    if (keys[14] - keys_old[14] >= rp_sens) {
      keysPressed[14] = 0;
    }
    if (keys_old[14] - keys[14] >= rp_sens) {  //d is pressed
      keysPressed[14] = 1;
    }
    if (keys[15] - keys_old[15] >= rp_sens) {
      keysPressed[15] = 0;
    }
    if (keys_old[15] - keys[15] >= rp_sens) {  //s is pressed
      keysPressed[15] = 1;
    }
    if (keys[16] - keys_old[16] >= rp_sens) {
      keysPressed[16] = 0;
    }
    if (keys_old[16] - keys[16] >= rp_sens) {  //a is pressed
      keysPressed[16] = 1;
    }
  } else {
    if (keys[8] >= actuation_point) {
      keysPressed[8] = 0;
    }
    if (keys[8] < actuation_point) {  //w is presssed
      keysPressed[8] = 1;
    }
    if (keys[14] >= actuation_point) {
      keysPressed[14] = 0;
    }
    if (keys[14] < actuation_point) {  //d is pressed
      keysPressed[14] = 1;
    }
    if (keys[15] >= actuation_point) {
      keysPressed[15] = 0;
    }
    if (keys[15] < actuation_point) {  //s is pressed
      keysPressed[15] = 1;
    }
    if (keys[16] >= actuation_point) {
      keysPressed[16] = 0;
    }
    if (keys[16] < actuation_point) {  //a is pressed
      keysPressed[16] = 1;
    }
  }

  if (keysPressed[16] == 1 && keysPressed[14] == 0) {
    keys_old[16] = keys[16];
    keys_old[14] = keys[14];
    Keyboard.release(key_names[14]);
    Keyboard.press(key_names[16]);
  }
  if (keysPressed[16] == 0 && keysPressed[14] == 1) {
    keys_old[16] = keys[16];
    keys_old[14] = keys[14];
    Keyboard.release(key_names[16]);
    Keyboard.press(key_names[14]);
  }
  if ((keysPressed[16] == 0 && keysPressed[14] == 0)) {
    keys_old[16] = keys[16];
    keys_old[14] = keys[14];
    Keyboard.release(key_names[16]);
    Keyboard.release(key_names[14]);
  }
  if (keysPressed[16] == 1 && keysPressed[14] == 1) {
    if (keysPressed_old[16] == 1) {
      Keyboard.release(key_names[16]);
      Keyboard.press(key_names[14]);
      keys_old[14] = keys[14];
    }
    if (keysPressed_old[14] == 1) {
      Keyboard.release(key_names[14]);
      Keyboard.press(key_names[16]);
      keys_old[16] = keys[16];
    }
  } else {
    keysPressed_old[16] = keysPressed[16];
    keysPressed_old[14] = keysPressed[14];
  }

  if (keysPressed[8] == 1 && keysPressed[15] == 0) {
    keys_old[8] = keys[8];
    keys_old[15] = keys[15];
    Keyboard.release(key_names[15]);
    Keyboard.press(key_names[8]);
  }
  if (keysPressed[8] == 0 && keysPressed[15] == 1) {
    keys_old[8] = keys[8];
    keys_old[15] = keys[15];
    Keyboard.release(key_names[8]);
    Keyboard.press(key_names[15]);
  }
  if ((keysPressed[8] == 0 && keysPressed[15] == 0)) {
    keys_old[8] = keys[8];
    keys_old[15] = keys[15];
    Keyboard.release(key_names[8]);
    Keyboard.release(key_names[15]);
  }
  if (keysPressed[8] == 1 && keysPressed[15] == 1) {
    if (keysPressed_old[8] == 1) {
      Keyboard.release(key_names[8]);
      Keyboard.press(key_names[15]);
      keys_old[15] = keys[15];
    }
    if (keysPressed_old[15] == 1) {
      Keyboard.release(key_names[15]);
      Keyboard.press(key_names[8]);
      keys_old[8] = keys[8];
    }
  } else {
    keysPressed_old[8] = keysPressed[8];
    keysPressed_old[15] = keysPressed[15];
  }
}

void menu_control() {
  CPU_SCORE = 0;
  PLAYER_SCORE = 0;
  Keyboard.release(0xB1);
  SOCD_location = 1;
  display.clear();
  display.setCursor(0, 10);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("Menu");
  display.setCursor(10, 20);
  display.print("Calibration");
  display.setCursor(10, 30);
  display.print("SOCD Settings");
  display.setCursor(10, 40);
  display.print("Wallpaper");
  display.setCursor(10, 50);
  display.print("Actuation");
  display.drawCircle(4, 22, 2, LED_WHITE_HIGH);
  display.show();
  byte location = 1;
  bool new_page = 0;
  bool on_same_page = 0;
  int arrow_timer = millis();
  int arrow_timer_old = arrow_timer;
  bool flash = 1;
  while (true) {
    newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      if (newPosition < oldPosition - 1) {
        oldPosition = newPosition;
        location++;
      }
      if (newPosition > oldPosition + 1) {
        oldPosition = newPosition;
        location--;
      }
      if (location == 0) {
        new_page = 1;
        location = 6;
      }
      if (location == 7) {
        new_page = 1;
        location = 1;
      }
      if (location == 1) {  //Calibration
        if (new_page) {
          display.fillRect(10, 20, 90, 50, LED_BLACK);
          display.setCursor(0, 10);
          display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
          display.setTextSize(1);
          display.setRotation(0);
          display.setTextColor(LED_CYAN_LOW);
          display.print("Menu");
          display.setCursor(10, 20);
          display.print("Calibration");
          display.setCursor(10, 30);
          display.print("SOCD Settings");
          display.setCursor(10, 40);
          display.print("Wallpaper");
          display.setCursor(10, 50);
          display.print("Key Actuation");
          display.drawLine(44, 15, 52, 15, LED_BLACK);
          display.drawLine(46, 14, 50, 14, LED_BLACK);
          display.drawLine(47, 13, 49, 13, LED_BLACK);
          display.drawLine(47, 12, 49, 12, LED_BLACK);
          display.drawLine(44, 15, 48, 11, LED_BLACK);
          display.drawLine(48, 11, 52, 15, LED_BLACK);
          new_page = 0;
        }
        on_same_page = 0;
        display.drawCircle(4, 22, 2, LED_WHITE_HIGH);
        display.drawCircle(4, 32, 2, LED_BLACK);
        display.drawCircle(4, 42, 2, LED_BLACK);
        display.drawCircle(4, 52, 2, LED_BLACK);
      }
      if (location == 2) {  //SOCD Settings
        display.drawCircle(4, 22, 2, LED_BLACK);
        display.drawCircle(4, 32, 2, LED_WHITE_HIGH);
        display.drawCircle(4, 42, 2, LED_BLACK);
        display.drawCircle(4, 52, 2, LED_BLACK);
      }
      if (location == 3) {  //Wallpaper Image
        display.drawCircle(4, 22, 2, LED_BLACK);
        display.drawCircle(4, 32, 2, LED_BLACK);
        display.drawCircle(4, 42, 2, LED_WHITE_HIGH);
        display.drawCircle(4, 52, 2, LED_BLACK);
        new_page = 0;
      }
      if (location == 4) {  //Actuation
        if (new_page && on_same_page) {
          display.fillRect(10, 20, 90, 50, LED_BLACK);
          display.setCursor(0, 10);
          display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
          display.setTextSize(1);
          display.setRotation(0);
          display.setTextColor(LED_CYAN_LOW);
          display.print("Menu");
          display.setCursor(10, 20);
          display.print("Calibration");
          display.setCursor(10, 30);
          display.print("SOCD Settings");
          display.setCursor(10, 40);
          display.print("Wallpaper");
          display.setCursor(10, 50);
          display.print("Actuation");
          display.drawLine(44, 15, 52, 15, LED_BLACK);
          display.drawLine(46, 14, 50, 14, LED_BLACK);
          display.drawLine(47, 13, 49, 13, LED_BLACK);
          display.drawLine(47, 12, 49, 12, LED_BLACK);
          display.drawLine(44, 15, 48, 11, LED_BLACK);
          display.drawLine(48, 11, 52, 15, LED_BLACK);
        }
        on_same_page = 0;
        new_page = 1;
        display.drawCircle(4, 22, 2, LED_BLACK);
        display.drawCircle(4, 32, 2, LED_BLACK);
        display.drawCircle(4, 42, 2, LED_BLACK);
        display.drawCircle(4, 52, 2, LED_WHITE_HIGH);
      }
      if (location == 5) {  //pong
        if (new_page && !on_same_page) {
          display.fillRect(10, 20, 90, 50, LED_BLACK);
          display.setTextColor(LED_CYAN_LOW);
          display.setCursor(10, 20);
          display.print("Pong");
          display.setCursor(10, 30);
          display.print("Quit");
          display.drawLine(44, 59, 52, 59, LED_BLACK);
          display.drawLine(46, 60, 50, 60, LED_BLACK);
          display.drawLine(47, 61, 49, 61, LED_BLACK);
          display.drawLine(47, 62, 49, 62, LED_BLACK);
          display.drawLine(44, 59, 49, 64, LED_BLACK);
          display.drawLine(47, 64, 52, 59, LED_BLACK);
          new_page = 0;
        }
        on_same_page = 1;
        new_page = 1;
        display.drawCircle(4, 22, 2, LED_WHITE_HIGH);
        display.drawCircle(4, 32, 2, LED_BLACK);
        display.drawCircle(4, 52, 2, LED_BLACK);
      }
      if (location == 6) {  //quit
        if (new_page && !on_same_page) {
          display.fillRect(10, 20, 90, 50, LED_BLACK);
          display.setTextColor(LED_CYAN_LOW);
          display.setCursor(10, 20);
          display.print("Pong");
          display.setCursor(10, 30);
          display.print("Quit");
          display.drawLine(44, 59, 52, 59, LED_BLACK);
          display.drawLine(46, 60, 50, 60, LED_BLACK);
          display.drawLine(47, 61, 49, 61, LED_BLACK);
          display.drawLine(47, 62, 49, 62, LED_BLACK);
          display.drawLine(44, 59, 49, 64, LED_BLACK);
          display.drawLine(47, 64, 52, 59, LED_BLACK);
          new_page = 0;
        }
        display.drawCircle(4, 22, 2, LED_BLACK);
        display.drawCircle(4, 32, 2, LED_WHITE_HIGH);
      }
      display.show();
    }
    arrow_timer = millis();
    if (arrow_timer - arrow_timer_old >= 750) {
      if (location < 5) {
        if (flash) {
          display.drawLine(44, 59, 52, 59, LED_WHITE_HIGH);
          display.drawLine(46, 60, 50, 60, LED_WHITE_HIGH);
          display.drawLine(47, 61, 49, 61, LED_WHITE_HIGH);
          display.drawLine(47, 62, 49, 62, LED_WHITE_HIGH);
          display.drawLine(44, 59, 49, 64, LED_WHITE_HIGH);
          display.drawLine(47, 64, 52, 59, LED_WHITE_HIGH);
          flash = 0;
        } else {
          display.drawLine(44, 59, 52, 59, LED_BLACK);
          display.drawLine(46, 60, 50, 60, LED_BLACK);
          display.drawLine(47, 61, 49, 61, LED_BLACK);
          display.drawLine(47, 62, 49, 62, LED_BLACK);
          display.drawLine(44, 59, 49, 64, LED_BLACK);
          display.drawLine(47, 64, 52, 59, LED_BLACK);
          flash = 1;
        }
      } else {
        if (flash) {
          display.drawLine(44, 15, 52, 15, LED_WHITE_HIGH);
          display.drawLine(46, 14, 50, 14, LED_WHITE_HIGH);
          display.drawLine(47, 13, 49, 13, LED_WHITE_HIGH);
          display.drawLine(47, 12, 49, 12, LED_WHITE_HIGH);
          display.drawLine(44, 15, 48, 11, LED_WHITE_HIGH);
          display.drawLine(48, 11, 52, 15, LED_WHITE_HIGH);
          flash = 0;
        } else {
          display.drawLine(44, 15, 52, 15, LED_BLACK);
          display.drawLine(46, 14, 50, 14, LED_BLACK);
          display.drawLine(47, 13, 49, 13, LED_BLACK);
          display.drawLine(47, 12, 49, 12, LED_BLACK);
          display.drawLine(44, 15, 48, 11, LED_BLACK);
          display.drawLine(48, 11, 52, 15, LED_BLACK);
          flash = 1;
        }
      }
      arrow_timer_old = arrow_timer;
    }
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  if (location == 1) {  //Calibration
    calibrate();
  }
  if (location == 2) {  //SOCD Settings
    socd_menu();
  }
  if (location == 3) {  //Wallpaper Image
    wallpaper();
  }
  if (location == 4) {  //Wallpaper Image
    actuation_settings();
  }
  if (location == 5) {  //Wallpaper Image
    pong_game();
  }
  if (location == 6) {  //quit
    if (wallpaper_num == 1) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)main_image, 96, 64);
    }
    if (wallpaper_num == 2) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)skull, 96, 64);
    }
    if (wallpaper_num == 3) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)rainbow, 96, 64);
    }
    if (wallpaper_num == 4) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)cats, 96, 64);
    }
    if (wallpaper_num == 5) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)tommyB, 96, 64);
    }
    if (wallpaper_num == 6) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)colorBars, 96, 64);
    }
    if (wallpaper_num == 7) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)ak47, 96, 64);
    }
    if (wallpaper_num == 8) {
      display.drawRGBBitmap(0, 0, (const uint16_t *)geometric, 96, 64);
    }
    bool should_commit = 0;
    if (SOCD != SOCD_num_saved) {
      EEPROM.write(96, SOCD);
      should_commit = 1;
    }
    if (wallpaper_num != wallpaper_num_saved) {
      EEPROM.write(97, wallpaper_num);
      should_commit = 1;
    }
    if (actuation != actuation_saved) {
      EEPROM.write(98, actuation);
      should_commit = 1;
    }
    if (rp_sens != rp_sens_saved) {
      EEPROM.write(99, rp_sens);
      should_commit = 1;
    }
    if (actuation_point != actuation_point_saved) {
      EEPROM.write(100, actuation_point);
      should_commit = 1;
    }
    if (should_commit) {
      EEPROM.commit();
    }
    display.show();
  }
}

void wallpaper() {
  wallpaper_num = 1;
  display.clear();
  display.setCursor(0, 0);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("Press knob to");
  display.setCursor(0, 10);
  display.print("continue");
  display.setCursor(0, 25);
  display.print("Select wallpaper");
  display.setCursor(0, 35);
  display.print("with another");
  display.setCursor(0, 45);
  display.print("knob press");
  display.show();
  while (true) {
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  display.drawRGBBitmap(0, 0, (const uint16_t *)main_image, 96, 64);
  display.show();
  while (true) {
    newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      if (newPosition < oldPosition - 1) {
        oldPosition = newPosition;
        wallpaper_num++;
      }
      if (newPosition > oldPosition + 1) {
        oldPosition = newPosition;
        wallpaper_num--;
      }
      if (wallpaper_num == 0) {
        wallpaper_num = 8;
      }
      if (wallpaper_num == 9) {
        wallpaper_num = 1;
      }
      if (wallpaper_num == 1) {
        display.drawRGBBitmap(0, 0, (const uint16_t *)main_image, 96, 64);
      }
      if (wallpaper_num == 2) {
        display.drawRGBBitmap(0, 0, (const uint16_t *)skull, 96, 64);
      }
      if (wallpaper_num == 3) {
        display.drawRGBBitmap(0, 0, (const uint16_t *)rainbow, 96, 64);
      }
      if (wallpaper_num == 4) {
        display.drawRGBBitmap(0, 0, (const uint16_t *)cats, 96, 64);
      }
      if (wallpaper_num == 5) {
        display.drawRGBBitmap(0, 0, (const uint16_t *)tommyB, 96, 64);
      }
      if (wallpaper_num == 6) {
        display.drawRGBBitmap(0, 0, (const uint16_t *)colorBars, 96, 64);
      }
      if (wallpaper_num == 7) {
        display.drawRGBBitmap(0, 0, (const uint16_t *)ak47, 96, 64);
      }
      if (wallpaper_num == 8) {
        display.drawRGBBitmap(0, 0, (const uint16_t *)geometric, 96, 64);
      }
      display.show();
    }
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
}

void socd_menu() {
  display.clear();
  display.setCursor(0, 10);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("SOCD Settings:");
  display.setCursor(10, 25);
  if (SOCD) {
    display.fillRect(8, 23, 26, 18, LED_GREEN_HIGH);
    display.setTextColor(LED_BLACK);
  } else {
    display.setTextColor(LED_CYAN_LOW);
  }
  display.setTextSize(2);
  display.print("On");
  display.setCursor(50, 25);
  if (!SOCD) {
    display.fillRect(48, 23, 38, 18, LED_RED_HIGH);
    display.setTextColor(LED_BLACK);
  } else {
    display.setTextColor(LED_CYAN_LOW);
  }
  display.setTextSize(2);
  display.print("Off");
  display.setCursor(34, 50);
  display.setTextColor(LED_CYAN_LOW);
  display.setTextSize(1);
  display.print("Quit");
  if (SOCD_location == 1) {  //On
    display.drawCircle(4, 32, 2, LED_WHITE_HIGH);
  }
  if (SOCD_location == 2) {  //Off
    display.drawCircle(44, 32, 2, LED_WHITE_HIGH);
  }
  if (SOCD_location == 3) {  //Quit
    display.drawCircle(28, 52, 2, LED_WHITE_HIGH);
  }
  display.show();
  while (true) {
    newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      if (newPosition < oldPosition - 1) {
        oldPosition = newPosition;
        SOCD_location++;
      }
      if (newPosition > oldPosition + 1) {
        oldPosition = newPosition;
        SOCD_location--;
      }
      if (SOCD_location == 0) {
        SOCD_location = 3;
      }
      if (SOCD_location == 4) {
        SOCD_location = 1;
      }
      if (SOCD_location == 1) {  //On
        display.drawCircle(4, 32, 2, LED_WHITE_HIGH);
        display.drawCircle(44, 32, 2, LED_BLACK);
        display.drawCircle(28, 52, 2, LED_BLACK);
      }
      if (SOCD_location == 2) {  //Off
        display.drawCircle(4, 32, 2, LED_BLACK);
        display.drawCircle(44, 32, 2, LED_WHITE_HIGH);
        display.drawCircle(28, 52, 2, LED_BLACK);
      }
      if (SOCD_location == 3) {  //Quit
        display.drawCircle(4, 32, 2, LED_BLACK);
        display.drawCircle(44, 32, 2, LED_BLACK);
        display.drawCircle(28, 52, 2, LED_WHITE_HIGH);
      }
      display.show();
    }
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  if (SOCD_location == 1) {  //Calibration
    SOCD = 1;
    socd_menu();
  }
  if (SOCD_location == 2) {  //SOCD Settings
    SOCD = 0;
    socd_menu();
  }
}

void volume_control() {
  newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    if (newPosition < oldPosition - 1) {
      oldPosition = newPosition;
      ConsumerControl.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
      delay(1);
      ConsumerControl.release();
    }
    if (newPosition > oldPosition + 1) {
      oldPosition = newPosition;
      ConsumerControl.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
      delay(1);
      ConsumerControl.release();
    }
  }
}

void actuation_settings() {
  if (actuation_location == 3) {  //Quit
    actuation_location = 1;
  }
  display.clear();
  display.setCursor(0, 10);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("Key Actuation:");
  display.setCursor(10, 25);
  if (!actuation) {
    display.fillRect(8, 23, 81, 10, LED_PURPLE_HIGH);
    display.setTextColor(LED_BLACK);
  } else {
    display.setTextColor(LED_CYAN_LOW);
  }
  display.print("Rapid Trigger");
  display.setCursor(10, 38);
  display.setTextSize(1);
  if (actuation) {
    display.fillRect(8, 36, 69, 10, LED_ORANGE_HIGH);
    display.setTextColor(LED_BLACK);
  } else {
    display.setTextColor(LED_CYAN_LOW);
  }
  display.print("Normal Mode");
  display.setCursor(34, 50);
  display.setTextColor(LED_CYAN_LOW);
  display.setTextSize(1);
  display.print("Quit");
  if (actuation_location == 1) {  //Rapid Trigger
    display.drawCircle(4, 27, 2, LED_WHITE_HIGH);
    display.drawCircle(4, 40, 2, LED_BLACK);
    display.drawCircle(28, 52, 2, LED_BLACK);
  }
  if (actuation_location == 2) {  //Actuation Point
    display.drawCircle(4, 27, 2, LED_BLACK);
    display.drawCircle(4, 40, 2, LED_WHITE_HIGH);
    display.drawCircle(28, 52, 2, LED_BLACK);
  }
  display.show();
  while (true) {
    newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      if (newPosition < oldPosition - 1) {
        oldPosition = newPosition;
        actuation_location++;
      }
      if (newPosition > oldPosition + 1) {
        oldPosition = newPosition;
        actuation_location--;
      }
      if (actuation_location == 0) {
        actuation_location = 3;
      }
      if (actuation_location == 4) {
        actuation_location = 1;
      }
      if (actuation_location == 1) {  //Rapid Trigger
        display.drawCircle(4, 27, 2, LED_WHITE_HIGH);
        display.drawCircle(4, 40, 2, LED_BLACK);
        display.drawCircle(28, 52, 2, LED_BLACK);
      }
      if (actuation_location == 2) {  //Actuation Point
        display.drawCircle(4, 27, 2, LED_BLACK);
        display.drawCircle(4, 40, 2, LED_WHITE_HIGH);
        display.drawCircle(28, 52, 2, LED_BLACK);
      }
      if (actuation_location == 3) {  //Quit
        display.drawCircle(4, 27, 2, LED_BLACK);
        display.drawCircle(4, 40, 2, LED_BLACK);
        display.drawCircle(28, 52, 2, LED_WHITE_HIGH);
      }
      display.show();
    }
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  if (actuation_location == 1) {
    actuation = 0;
    rapid_trigger_menu();
  }
  if (actuation_location == 2) {
    actuation = 1;
    actutaion_point_menu();
  }
}

void pong_game() {
  int slowmo_start = millis();
  int press_timer = millis();
  int press_timer_old = press_timer - 750;
  bool press = 1;
  display.fillScreen(BLACK);
  display.drawBitmap(3, 0, pong, 89, 24, GREEN);
  display.drawBitmap(10, 26, game, 75, 26, RED);
  delay(1000);

  while (true) {
    press_timer = millis();
    if (press_timer - press_timer_old >= 750) {
      if (press) {
        display.setTextColor(LED_CYAN_LOW);
        display.setCursor(20, 57);
        display.print("Press Knob");
        display.show();
        press = 0;
      } else {
        display.setTextColor(LED_BLACK);
        display.setCursor(20, 57);
        display.print("Press Knob");
        display.show();
        press = 1;
      }
      press_timer_old = press_timer;
    }
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  unsigned long start = millis();


  display.fillScreen(BLACK);
  drawCourt();

  while (millis() - start < 2000)
    ;
  ball_update = millis();
  paddle_update = ball_update;
  ball_x = random(35, 55);
  ball_y = random(3, 63);

  while (true) {
    unsigned long time = millis();
    static bool up_state = false;
    static bool down_state = false;

    newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      if (newPosition < oldPosition - 1) {
        oldPosition = newPosition;
        down_state = true;
      }
      if (newPosition > oldPosition + 1) {
        oldPosition = newPosition;
        up_state = true;
      }
    }

    if (resetBall) {
      ball_x = random(25, 70);
      ball_y = random(3, 63);
      do {
        ball_dir_x = random(-1, 2);
      } while (ball_dir_x == 0);

      do {
        ball_dir_y = random(-1, 2);
      } while (ball_dir_y == 0);


      resetBall = false;
      slowmo_start = millis();
    }
    int slowmo = millis();
    if (slowmo - slowmo_start > 21000) {
      BALL_RATE = 10;
      slowmo++;
    } else if (slowmo - slowmo_start > 18000) {
      BALL_RATE = 14;
      slowmo++;
    } else if (slowmo - slowmo_start > 15000) {
      BALL_RATE = 18;
      slowmo++;
    } else if (slowmo - slowmo_start > 12000) {
      BALL_RATE = 22;
      slowmo++;
    } else if (slowmo - slowmo_start > 8000) {
      BALL_RATE = 26;
      slowmo++;
    } else if (slowmo - slowmo_start > 5000) {
      BALL_RATE = 30;
      slowmo++;
    } else {
      BALL_RATE = 34;
    }
    if (time > ball_update && gameIsRunning) {
      int new_x = ball_x + ball_dir_x;
      int new_y = ball_y + ball_dir_y;

      // Check if we hit the vertical walls
      if (new_x == 0)  //Player Gets a Point
      {
        PLAYER_SCORE++;
        if (PLAYER_SCORE == MAX_SCORE) {
          gameOver();
        } else {
          showScore();
        }
      }

      // Check if we hit the vertical walls
      if (new_x == 95)  //CPU Gets a Point
      {
        CPU_SCORE++;
        if (CPU_SCORE == MAX_SCORE) {
          gameOver();
        } else {
          showScore();
        }
      }

      // Check if we hit the horizontal walls.
      if (new_y <= 0 || new_y >= 63) {
        ball_dir_y = -ball_dir_y;
        new_y += ball_dir_y + ball_dir_y;
      }

      // Check if we hit the CPU paddle
      if (new_x == CPU_X && new_y >= cpu_y && new_y <= cpu_y + PADDLE_HEIGHT + 5) {
        ball_dir_x = -ball_dir_x;
        new_x += ball_dir_x + ball_dir_x;
        if (random(1, 9) == 5) {
          int ran = random(-2, 3);
          while (ran == -ball_dir_y) {
            ran = random(-2, 3);
          }
          ball_dir_y = ball_dir_y + ran;
        }
      }

      // Check if we hit the player paddle
      if (new_x == PLAYER_X
          && new_y >= player_y
          && new_y <= player_y + PADDLE_HEIGHT) {
        ball_dir_x = -ball_dir_x;
        new_x += ball_dir_x + ball_dir_x;
        if (random(1, 9) == 5) {
          int ran = random(-2, 3);
          while (ran == -ball_dir_y) {
            ran = random(-2, 3);
          }
          ball_dir_y = ball_dir_y + ran;
        }
      }

      display.drawPixel(ball_x, ball_y, BLACK);
      display.drawPixel(new_x, new_y, WHITE);
      ball_x = new_x;
      ball_y = new_y;

      ball_update += BALL_RATE;
    }

    if (time > paddle_update && gameIsRunning) {
      paddle_update += PADDLE_RATE;

      // CPU paddle
      display.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT + 5, BLACK);
      const uint8_t half_paddle = PADDLE_HEIGHT + 5 >> 1;
      if (cpu_y + half_paddle > ball_y) {
        cpu_y -= 2;
      }
      if (cpu_y + half_paddle < ball_y) {
        cpu_y += 2;
      }
      if (cpu_y < 1) cpu_y = 1;
      if (cpu_y + PADDLE_HEIGHT + 5 > 63) cpu_y = 63 - (PADDLE_HEIGHT + 5);
      display.drawFastVLine(CPU_X, cpu_y, PADDLE_HEIGHT + 5, RED);

      // Player paddle
      display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, BLACK);
      if (up_state) {
        player_y -= 3;
      }
      if (down_state) {
        player_y += 3;
      }
      up_state = down_state = false;
      if (player_y < 1) player_y = 1;
      if (player_y + PADDLE_HEIGHT > 63) player_y = 41;
      display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, GREEN);
    }
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
}

void drawCourt() {
  display.drawRect(0, 0, 96, 64, WHITE);
}

void gameOver() {
  int press_timer = millis();
  int press_timer_old = press_timer - 750;
  bool press = 1;
  gameIsRunning = false;
  display.fillScreen(BLACK);
  drawCourt();
  if (PLAYER_SCORE > CPU_SCORE) {
    display.setCursor(5, 4);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.print("You Won");
  } else {
    display.setCursor(5, 4);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.print("CPU WON");
  }

  display.setCursor(20, 30);
  display.setTextColor(RED);
  display.setTextSize(3);
  display.print(String(CPU_SCORE));

  display.setCursor(60, 30);
  display.setTextColor(GREEN);
  display.setTextSize(3);
  display.print(String(PLAYER_SCORE));

  while (true) {
    press_timer = millis();
    if (press_timer - press_timer_old >= 750) {
      if (press) {
        display.setTextSize(1);
        display.setTextColor(LED_CYAN_LOW);
        display.setCursor(20, 45);
        display.print("Press Knob");
        display.show();
        press = 0;
      } else {
        display.setTextSize(1);
        display.setTextColor(LED_BLACK);
        display.setCursor(20, 45);
        display.print("Press Knob");
        display.show();
        press = 1;
      }
      press_timer_old = press_timer;
    }
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  display.setTextSize(1);
  display.setTextColor(LED_BLACK);
  display.setCursor(20, 45);
  display.print("Press Knob");
  display.show();
  gameIsRunning = true;

  CPU_SCORE = PLAYER_SCORE = 0;

  unsigned long start = millis();
  display.fillScreen(BLACK);
  drawCourt();
  while (millis() - start < 2000)
    ;
  ball_update = millis();
  paddle_update = ball_update;
  gameIsRunning = true;
  resetBall = true;
}

void showScore() {
  int press_timer = millis();
  int press_timer_old = press_timer - 750;
  bool press = 1;
  gameIsRunning = false;
  display.fillScreen(BLACK);
  drawCourt();

  display.setCursor(15, 4);
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.print("Score");

  display.setCursor(20, 30);
  display.setTextColor(RED);
  display.setTextSize(3);
  display.print(String(CPU_SCORE));

  display.setCursor(60, 30);
  display.setTextColor(GREEN);
  display.setTextSize(3);
  display.print(String(PLAYER_SCORE));

  delay(2000);
  unsigned long start = millis();

  display.fillScreen(BLACK);
  drawCourt();

  while (true) {
    press_timer = millis();
    if (press_timer - press_timer_old >= 750) {
      if (press) {
        display.setTextSize(1);
        display.setTextColor(LED_CYAN_LOW);
        display.setCursor(20, 25);
        display.print("Press Knob");
        display.show();
        press = 0;
      } else {
        display.setTextSize(1);
        display.setTextColor(LED_BLACK);
        display.setCursor(20, 25);
        display.print("Press Knob");
        display.show();
        press = 1;
      }
      press_timer_old = press_timer;
    }
    escape.update();
    if (escape.fell()) {
      break;
    }
  }

  display.setTextSize(1);
  display.setTextColor(LED_BLACK);
  display.setCursor(20, 25);
  display.print("Press Knob");
  display.show();

  ball_update = millis();
  paddle_update = ball_update;
  gameIsRunning = true;
  resetBall = true;
}

void rapid_trigger_menu() {
  display.clear();
  display.setCursor(0, 10);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("Rapid Trigger:");
  display.setCursor(10, 20);
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("Please Set");
  display.setCursor(10, 30);
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("Sensistivity");
  display.setTextSize(2);
  display.setRotation(0);
  display.setCursor(40, 40);
  display.setTextColor(LED_PURPLE_HIGH);
  display.print(rp_sens);
  display.show();

  while (true) {
    newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      display.setCursor(40, 40);
      display.setTextColor(LED_BLACK);
      display.print(rp_sens);
      if (newPosition < oldPosition - 1) {
        oldPosition = newPosition;
        rp_sens++;
      }
      if (newPosition > oldPosition + 1) {
        oldPosition = newPosition;
        rp_sens--;
      }
      if (rp_sens == 11) {
        rp_sens = 10;
      }
      if (rp_sens == 0) {
        rp_sens = 1;
      }
      display.setTextSize(2);
      display.setRotation(0);
      display.setCursor(40, 40);
      display.setTextColor(LED_PURPLE_HIGH);
      display.print(rp_sens);
      display.show();
    }
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  actuation_settings();
}

void actutaion_point_menu() {
  float decimal_actuation_point = (11 - actuation_point) * 0.35;
  display.clear();
  display.setCursor(0, 10);
  display.setTextWrap(true);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("Normal Mode:");
  display.setCursor(10, 20);
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("Please Set");
  display.setCursor(5, 30);
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(LED_CYAN_LOW);
  display.print("Actuation Point");
  display.setTextSize(2);
  display.setRotation(0);
  display.setCursor(20, 44);
  display.setTextColor(LED_ORANGE_HIGH);
  display.print(decimal_actuation_point, 2);
  display.setTextSize(1);
  display.setRotation(0);
  display.setCursor(68, 50);
  display.setTextColor(LED_ORANGE_HIGH);
  display.print("mm");
  display.show();

  while (true) {
    newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      display.setCursor(20, 44);
      display.setTextColor(LED_BLACK);
      display.print(decimal_actuation_point, 2);
      if (newPosition < oldPosition - 1) {
        oldPosition = newPosition;
        actuation_point--;
      }
      if (newPosition > oldPosition + 1) {
        oldPosition = newPosition;
        actuation_point++;
      }
      if (actuation_point == 11) {
        actuation_point = 10;
      }
      if (actuation_point == 0) {
        actuation_point = 1;
      }
      decimal_actuation_point = (11 - actuation_point) * 0.35;
      display.setTextSize(2);
      display.setRotation(0);
      display.setCursor(20, 44);
      display.setTextColor(LED_ORANGE_HIGH);
      display.print(decimal_actuation_point, 2);
      display.show();
    }
    escape.update();
    if (escape.fell()) {
      break;
    }
  }
  actuation_settings();
}