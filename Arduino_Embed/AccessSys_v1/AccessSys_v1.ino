/************************************************
 * Copyright: Wi-Tech group, all rights reserved. 
 * File Name: AccessSys
 * File No.: 001
 * 
 * Project Name: Wi-Eye: smart access control sys-
 *    tem via gait recognition using Wi-Fi signal. 
 * Design Document(s): Wi-Eye-无线之瞳.docx
 * Main Algorithm: DES, LSM(Limited State Machine)
 * Hardware Ports: for WEMOS LOLIN D1 mini
 *    D0 - Relay enable
 *    D1 - I2C SCL
 *    D2 - I2C SDA
 *    D3 - Fingerprint Tx
 *    D4 - Fingerprint Rx
 *    D5 - SPI SCK (Serial Clock)
 *    D6 - SPI MISO (Master In Slave Out)
 *    D7 - SPI MOSI (Master Out Slave In)
 *    D8 - SPI SS (Slave Select)
 *    A0 - Speaker Signal
 * Sub-functions: 
 *    None.
 * Developing Resume: 
 *    2019.04.23 10:30 file established
 * Designer: IssacRunmin
 * Design Date: 2019.04.23
 * Reviewer: 
 * Reviewer Date:
 * Modification Record: 
 *    1)  Modifier: 
 *        Date: 
 *        Number: 
 *        Content: 
 *    2)  Modifier:
 *        Date: 
 *        Number: 
 *        Content: 
 * Abstract: This Arduino project is a 
 * Version: v0.1
 * Author: Issacrunmin
 * Update Time: 2019.04.23 10:38
 ************************************************/

// External library
#include <DES.h>
#include <mpr121.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
// Board library
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <math.h>
// Define

#define MQTT_PORT 1883              // MQTT Server port
#define MQTT_IN_TOPIC strcat("WiEye", DEVICE_STR)
#define Keypad_DEVID 0x5B           // Keypad Device ID
#define OLED_DEVID 0x3C             // OLED Device ID
#define PIN_SCL D1
#define PIN_SDA D2
#define PIN_RX D4   // D4
#define PIN_TX D3   // D3
#define PIN_SCK D5    // D5
#define PIN_MISO D6   // D6
#define PIN_MOSI D7   // D7
#define PIN_SS D8     // D8
#define RST_MFRC522   UINT8_MAX         // MFRC522 Reset pin
#define RST_OLED -1                 // OLED Reset pin
#define PIN_RELAY D0
#define PIN_SPEAKER D3
#define DEBUG     false              // Program debug
#define CYCLE     5                 // times, LSM goto 0 when it come to 30
#define INTERVAL  10                // ms, main cycle time
#define EEPROM_CAPACITY 2048        // byte, use for store ID
#define KEY_DUR 60                  // ms, press down key for 300ms to activate
#define OPEN_DUR 3000               // ms, door open for 3000ms 
#define WIFI_ENCRYPT false          // encrypt the content when publish 
#define LOG_TOPIC "WiEyeClient"     // Log on Topic, for sever to 
#define KEY_EN true
#define OLED_EN true
#define FINGER_EN true
#define READER_EN true
#define WIFI_EN true
#define TIC_EN true
#define CARD_AUTH false
#if DEBUG
  #define PRINT(str) Serial.println(str);
#else
  #define PRINT(str)  ;
#endif
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
// Global variables:
union IDbyte{
  uint32 Int;
  byte Byte[4];
};
union CIDbyte{
  uint64 Int;
  byte Byte[8];
};
const IDbyte DEVID = {31415};
const byte DESkey[8] = {0x3b, 0x38, 0x98, 0x37, DEVID.Byte[0], DEVID.Byte[1],
    DEVID.Byte[2], DEVID.Byte[3]};
const IDbyte DEVPW = {31415};
const IDbyte AuthCard = {31415};
char DEVICE_STR[9];
SoftwareSerial mySerial(PIN_RX,PIN_TX);
// Fingerprint (Rx,Tx) port
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
DES des;
WiFiClient espClient;
PubSubClient client(espClient);
Ticker ticker;
Adafruit_SSD1306 display(RST_OLED);
const char ASCII[17]="0123456789ABCDEF";
MFRC522 Reader(PIN_SS,RST_MFRC522); // RFID Reader obj

uint32 KeyNum = 0;                  // just Task_Pad() modify
int Key = 0;
static uint32 PW = 0;

byte UserCount = 0;
int PW_TmpI = 0,PW_Tmp;             // just Used in Main() and MQTT_Callback()
uint16_t Finger_Threshold = 50;     // used in Task_Finger() and MQTT_Callback()
uint32 ticTime;                     // just used in tic() and toc()
uint32 StrTime;
CIDbyte Card;                       // just used in Main() and Reader() task
CIDbyte CardAuth;                   // used in Main and wifi
bool Confirm = false;               // Task_Pad() and Task_Main()
int32 StrFade = -1;                 // just use in OLED_Fade()
String PublishContent = "";
int Finger_Idx = 0;
struct payload{
    int len = 16;
    int slen = 0;
    String Content = "";
    uint8_t *p = (uint8_t *)malloc(16);
}PubPayload, SubPayLoad;
String OLEDStr = "";
// States: 0: disable;
int FingerS = 0;
int KeyS = 0;
int OLEDS = 0;
int RelayS = 0;
int ReaderS = 0;
int State = 0;
int WiFiS = 0;
/*************
 * 0: Initialize
 * 1: Normal state. 
 * 2: Lowpower state.
 * 3: Safe state
 * 4: Door open state
 *************/
static const unsigned char PROGMEM LOGO_wifi[] = 
{0x3C,0x42,0x81,0x18,0x24,0x00,0x18,0x00};
static const unsigned char PROGMEM LOGO_nowifi[] = 
{0x3D,0x42,0x85,0x18,0x34,0x20,0x58,0x80};
static const unsigned char PROGMEM LOGO_finger[] = 
{0x00,0x10,0x10,0x1C,0x5E,0x3E,0x1C,0x1C};
static const unsigned char PROGMEM LOGO_nofinger[] = 
{0x01,0x12,0x14,0x1C,0x5E,0x3E,0x5E,0x9C};
static const unsigned char PROGMEM LOGO_rfid[] = 
{0x08,0x1C,0x22,0x5B,0xDA,0x44,0x38,0x10};
static const unsigned char PROGMEM LOGO_norfid[] = 
{0x09,0x1E,0x26,0x5B,0xDA,0x64,0x78,0x90};
static const unsigned char PROGMEM LOGO_keypad[] = 
{0x3E,0x41,0x6B,0x41,0x55,0x41,0x6B,0x41};
static const unsigned char PROGMEM LOGO_nokeypad[] = 
{0x7D,0x82,0xD6,0x8A,0xBA,0xA2,0xD6,0x82};
static const unsigned char PROGMEM LOGO_battery3[] = 
{0x3F,0xFF,0x60,0x01,0xDD,0xDD,0x9D,0xDD,0xDD,0xDD,0x60,0x01,0x3F,0xFF,0x00,0x00};

static const unsigned char PROGMEM LOGO_main[] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x01,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xFF,0xFF,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xE1,0x07,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,0x1F,0xF0,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xFF,0xF8,0xDF,0xFC,0x1F,0xFF,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x03,0xFF,0xE1,0xCF,0xFE,0x07,0xFF,0x80,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x07,0xFF,0x83,0xAF,0xFF,0x01,0xFF,0xC0,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0F,0xFE,0x03,0x67,0xFF,0x80,0xFF,0xF0,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x1F,0xF8,0x00,0xC7,0xFF,0xC0,0x3F,0xF8,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x3F,0xF0,0x06,0x36,0xFF,0xC0,0x1F,0xFC,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x7F,0xE0,0x07,0x20,0x1F,0xE0,0x07,0xFE,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xFF,0xC0,0x0F,0x88,0x0F,0xE0,0x03,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xFF,0x00,0x0F,0xE7,0xC7,0xE0,0x01,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x03,0xFE,0x00,0x0F,0xC7,0xE7,0xE0,0x00,0xFF,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x07,0xFC,0x00,0x0F,0xCF,0xE7,0xF0,0x00,0x7F,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x0F,0xF8,0x00,0x0F,0xCF,0xF7,0xF0,0x00,0x3F,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x07,0xF8,0x00,0x0F,0xCF,0xF7,0xF0,0x00,0x0F,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x07,0xFC,0x00,0x0F,0xCF,0xE7,0xF0,0x00,0x07,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x03,0xFE,0x00,0x0F,0xC7,0xE7,0xE0,0x03,0xFE,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xFF,0x00,0x0F,0xE7,0xC7,0xE0,0x0F,0xFE,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xFF,0xC0,0x0F,0xF0,0x0F,0xE0,0x1F,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x7F,0xE0,0x07,0xF8,0x3F,0xE0,0x3E,0x0F,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x3F,0xF0,0x07,0xFF,0xFF,0xC0,0x3C,0xE7,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x1F,0xFC,0x03,0xFF,0xFF,0xC0,0x7D,0xF7,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0F,0xFE,0x03,0xFF,0xFF,0x80,0x7D,0xF7,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x07,0xFF,0x81,0xFF,0xFF,0x01,0x7D,0xF7,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x03,0xFF,0xE0,0xFF,0xFE,0x07,0x78,0x03,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xFF,0xF8,0x7F,0xFC,0x1F,0x78,0x43,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,0x1F,0xF0,0xFF,0xF8,0x43,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xE1,0x07,0xFF,0xF8,0x43,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xFF,0xFF,0xFF,0x38,0x43,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0xFF,0xFF,0x38,0x03,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x01,0xFF,0xFF,0xFF,0xFF,0x1F,0xFF,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xFC,0x1F,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xFF,0xF0,0x0F,0xFE,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0x80,0x03,0xFC,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const unsigned char PROGMEM LOGO_close1[] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xFF,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x3F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xFF,0xFF,0xFF,0xE3,0xF3,0xFF,0xFF,0xFF,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x03,0xFF,0xFF,0x87,0xE7,0xF3,0xF9,0xFF,0xFF,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xFF,0xF0,0x07,0xE7,0xFB,0xF8,0x0F,0xFF,0xF0,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xFE,0x00,0x07,0xE7,0xFB,0xF8,0x00,0x7F,0xF0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x7E,0x00,0x07,0xE7,0xF3,0xF8,0x00,0x03,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x3F,0x00,0x07,0xE3,0xF3,0xF0,0x01,0xFF,0x40,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x1F,0x80,0x07,0xF3,0xE3,0xF0,0x07,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x07,0xE0,0x07,0xF8,0x07,0xF0,0x0F,0xFF,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x03,0xF0,0x03,0xFC,0x1F,0xF0,0x1F,0x07,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x01,0xF8,0x03,0xFF,0xFF,0xE0,0x1E,0x73,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xFE,0x01,0xFF,0xFF,0xE0,0x3E,0xFB,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x3F,0x01,0xFF,0xFF,0xC0,0x3E,0xFB,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1F,0xC0,0xFF,0xFF,0x80,0xBE,0xFF,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x0F,0xF0,0x7F,0xFF,0x03,0xBC,0x01,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x03,0xFC,0x3F,0xFE,0x0F,0xBC,0x21,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x01,0xFF,0x0F,0xF8,0x7F,0xFC,0x21,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xF0,0x83,0xFF,0xFC,0x21,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xFF,0xF8,0x1C,0x21,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xFF,0xE0,0x1C,0x01,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0x80,0x0F,0xFF,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0x00,0x0F,0xFF,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xFE,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const unsigned char PROGMEM LOGO_close2[] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,0x3F,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xFF,0xF8,0x00,0x00,0x00,0x00,0x07,0xFF,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xFF,0xFF,0xC0,0x00,0x00,0x00,0xFF,0xFF,0xF0,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xFF,0xFF,0xFF,0x00,0x00,0x3F,0xFF,0xFF,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x3F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x1F,0x87,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x07,0xE0,0x3F,0xFF,0xFF,0xFF,0x8F,0xFF,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x03,0xF0,0x03,0xFE,0x3F,0xF0,0x1F,0x07,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x01,0xF8,0x03,0xFF,0x7F,0xE0,0x1E,0x73,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xFE,0x01,0xFF,0xFF,0xE0,0x3E,0xFB,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x3F,0x01,0xFF,0xFF,0xC0,0x3E,0xFF,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x1F,0xC0,0xFF,0xFF,0x80,0xFE,0xFF,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x0F,0xF0,0x7F,0xFF,0x03,0xFC,0x01,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x03,0xFC,0x3F,0xFE,0x0F,0xFC,0x21,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x01,0xFF,0x0F,0xF8,0x7F,0xFC,0x21,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xF0,0x83,0xFC,0x3C,0x21,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xFF,0xF8,0x1C,0x21,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xFF,0xE0,0x1C,0x01,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0x80,0x0F,0xFF,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0x00,0x0F,0xFF,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xFE,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static const unsigned char PROGMEM LOGO_closed[] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xF0,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0xFF,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xFF,0xF0,0x00,0x00,0x00,0x00,0x03,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x01,0xFF,0xF8,0x00,0x00,0x00,0x00,0x0F,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x03,0xFF,0xFE,0x00,0x00,0x00,0x00,0x3F,0xFF,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x07,0xF3,0xFF,0x80,0x00,0x00,0x00,0xC1,0xFF,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x0F,0xE1,0xFF,0xE0,0x00,0x00,0x03,0x9C,0xFF,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x0F,0xC0,0xFF,0xF8,0x00,0x00,0x0F,0xBE,0xFF,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x0F,0x81,0xFF,0xFF,0x00,0x00,0x3F,0xBE,0xFF,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x07,0x03,0xFF,0xFF,0xC0,0x01,0xFF,0xFE,0xFF,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0xFF,0x3F,0xFF,0xFC,0x01,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0F,0xE3,0xFF,0xFF,0xFF,0xFF,0xFC,0x21,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x1F,0xC1,0xFF,0xFF,0xFF,0xFF,0xFC,0x21,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x1F,0x81,0xFF,0xFF,0xFF,0xFF,0xBC,0x21,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x1F,0x03,0xFF,0xFF,0xFF,0xFF,0xDC,0x21,0xE0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x0E,0x07,0xFF,0xFF,0xFF,0xEF,0xDC,0x01,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x0F,0xE3,0xFF,0xFF,0x87,0xEF,0xFF,0xC0,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x0F,0xC1,0xFF,0xFF,0x83,0xEF,0xFF,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x0F,0x83,0xF0,0x0F,0xC3,0xE7,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x07,0x03,0xF0,0x07,0xC1,0xC1,0xFE,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xE0,0x07,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xC0,0x03,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

void setup() {
    // put your setup code here, to run once:
    byte Err;
    #if DEBUG
        Serial.begin(9600);
//        while(!Serial) {delay(20);}
        Serial.println("WiEye_hello world");
        delay(500);
        Serial.println("WiEye_Init");
    #endif
    Device_Initialize();
    delay(500);
}




/******************
 * Function: tic() and toc()
 * Discription: log the time in mircosecond(us)
 * Input: void
 * Output: void
 * Global variables: ticTime
 ******************/
 void tic(){
    #if TIC_EN
    ticTime = micros();
    #endif
 }
 uint32 toc(){
    #if TIC_EN
    return micros() - ticTime;
    #else
    return 0;
    #endif
    
 }

/******************
 * Function: Device_Initialize()
 * Discription: initialize devices, including Keypad, OLED, 
 *    MFRC522, WiFi, etc.
 * Input: void
 * Output: void
 * Global variables Change: DEVICE_STR
 * 
 ******************/
void Device_Initialize(){
    
    int FailedCount,i;
    IDbyte DevID_t;
    uint32 tmp;
    byte Err;
    PRINT("Device Checking")
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);
    #if KEY_EN || OLED_EN
      Wire.begin(PIN_SDA,PIN_SCL); // SDA SCL
    #endif
    #if OLED_EN
        Wire.beginTransmission(OLED_DEVID);
        Err = Wire.endTransmission();
        if (Err == 0){
            OLEDS = 1;
            PRINT("OLED I2C: Find")
        }
        #if DEBUG
        else if (Err == 4)
                Serial.println("OLED I2C: Unknow Error");
            else
                Serial.println("OLED I2C: No Connection");
        #endif
    #endif
    if (OLEDS == 1){ // show logo and state
      tic();
      display.begin(SSD1306_SWITCHCAPVCC, OLED_DEVID);
      display.clearDisplay();
      display.drawBitmap(0, 16, LOGO_main, 128, 48, 1);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(48,0);
      display.print("Init..");
      display.display();
      PRINT("Init OLED:")
      PRINT(toc())
    }
    Task_Pad();
    tmp = DEVID.Int;
    for  (i = 7;i >= 0; i--){
        DEVICE_STR[i] = ASCII[tmp & 0x0F];
        tmp = tmp >> 4;
    }
    DEVICE_STR[8] = '\0';
    PRINT("Device ID:")
    PRINT(MQTT_IN_TOPIC)   
    EEPROM.begin(EEPROM_CAPACITY);
    EEPROM_Read(0, 4, DevID_t.Byte);
//    for (i = 0; i < 4 && ID[i] == DEVID.Byte[i]; i++) ;
    if (DevID_t.Int != DEVID.Int) {
        byte check_sum;
        PRINT("Initialize EEPROM...")
        check_sum = 0;
        for (i = 0; i < 4; i++){
            EEPROM.write(i, DEVID.Byte[i]);
            check_sum += DEVID.Byte[i];
        }
        for (i = 8; i < 128; i++)
            EEPROM.write(i,0);
        EEPROM.write(4, 0);
        EEPROM.write(7,check_sum);
        EEPROM.commit();
    }
    else{
        UserCount = EEPROM.read(4);
        PRINT("User Count: ")
        PRINT(UserCount)
    }
    
    Task_Finger();
    Task_Reader();
    Task_WiFi();
    OLED_Display_Mode();
    if (OLEDS){
      display.setCursor(48,0);
      display.setTextColor(WHITE, BLACK);
      display.print("Ready.");
      display.display();
    }
    State = 1;
}

/******************
 * Function: 
 * Discription:
 * Input: topic , payload, length
 * Output: void
 * Global Variables:
 * 
 ******************/
void callback(char* topic, byte* payload, unsigned int length){
    if (strcmp(topic,DEVICE_STR)){
        CIDbyte CardIn, CardTmp, Card_Info;
        int i,j;
        byte tmp, ACState, NumIn;
        PRINT("In Message");
//        for (i = 0; i < 16; i++)
//            if (DEBUG){
//                Serial.print(payload[i]);
//                Serial.print(" ");
//            }
        for (i = 0; i < 8; i++){
            tmp = payload[2*i];
            if (tmp > 64) tmp -= 55;
            else tmp -= 48;
            CardIn.Byte[i] = tmp;
            CardIn.Byte[i] <<= 4;
            tmp = payload[2*i+1];
            if (tmp > 64) tmp -= 55;
            else tmp -= 48;
            CardIn.Byte[i] += tmp;
            PRINT(CardIn.Byte[i]);
        }
        ACState = payload[17] - '0';
        ACState <<=4;
        ACState += payload[18] - '0';
        NumIn = payload[19] - '0';
        NumIn <<=4;
        NumIn += payload[20] - '0';
        for (i = 0; i < UserCount; i++){
            EEPROM_Read((int)(128 + i * 16), 8, CardTmp.Byte);
            EEPROM_Read((int)(128 + i * 16 + 8), 2, Card_Info.Byte);
            if (CardTmp.Int == CardIn.Int || ((Card_Info.Byte[0] & 0x01) == 0))
                break;
        }
        switch (ACState & 0xF0){
            case 0x40: 
                if (i < UserCount){
                    PRINT("WiFi Auth")
                    WiFiS = 5;
                    CardAuth.Int = CardIn.Int;
        //            WiFi_Publish(CardAuth, 0x40);
                }
                else{
                    PRINT("Invaliad card");
                    OLEDStr = "WiFi Card F";
                }
            break;
            case 0x50: 
                if (i < UserCount){
                    Card_Info.Byte[0] |= 0x01;
                    EEPROM_write((int)(128 + i * 16 + 8), 1, Card_Info.Byte[0]);
                    PRINT("Card state change!")
                }
            break;
            default: PRINT("Unknown ACState, Aborting: #") PRINT(ACState);
        }
    }
    else{
        PRINT("Invalid topic");
        PRINT(topic);
    }
}

/******************
 * Function: 
 * Discription:
 * Input: void
 * Output: void
 * Global Variables: WiFiS, FingerS, 
 * 
 ******************/
void OLED_Fade(int32 dur){
    StrTime = millis();
    StrFade = dur;
}
void OLED_Display_Mode(){
    static bool FingerS_C = 0, ReaderS_C = 0, KeyS_C = 0, shink = 0;
    static int WiFiS_C = 0;
    static int State_C = 0;
    static String OLEDStr_C = "";
    static uint32 StrDur = 0;
    if (OLEDS == 1){
        display.setTextSize(2);
        display.setTextColor(WHITE, BLACK);
        display.setCursor(0, 32);
        if (!shink)
        display.print(".");
        else
        display.print(" ");
        shink = !shink;
        display.display();
    }
    if ((OLEDS == 1)&&(WiFiS_C != WiFiS || FingerS_C != (FingerS>=1) ||
        ReaderS_C != (ReaderS>=1) || KeyS_C != (KeyS>=1) || State_C != State)){
        PRINT("OLED: State changed")
//        PRINT(WiFiS);PRINT(WiFiS_C);
        display.setTextSize(1);
        display.setTextColor(WHITE, BLACK);
        display.setCursor(0, 0);
        display.print("                ");
        #if DEBUG
        if (WiFiS == 0 || WiFiS == 4)
            display.drawBitmap(94, 0, LOGO_nowifi, 8, 8, 1);
        else
            display.drawBitmap(94, 0, LOGO_wifi, 8, 8, 1);
        display.setCursor(102, 0);
        display.print(WiFiS);
        if (FingerS)
            display.drawBitmap(0, 0, LOGO_finger, 8, 8, 1);
        else
            display.drawBitmap(0, 0, LOGO_nofinger, 8, 8, 1);
        if (ReaderS)
            display.drawBitmap(10, 0, LOGO_rfid, 8, 8, 1);
        else
            display.drawBitmap(10, 0, LOGO_norfid, 8, 8, 1);
        if (KeyS)
            display.drawBitmap(20, 0, LOGO_keypad, 8, 8, 1);
        else
            display.drawBitmap(20, 0, LOGO_nokeypad, 8, 8, 1);
        #else
        
        if (WiFiS == 0 || WiFiS == 4){
            display.drawBitmap(94, 0, LOGO_nowifi, 8, 8, 1);
            display.setCursor(102, 0);
            display.print("!");
        }
        else
            display.drawBitmap(94, 0, LOGO_wifi, 8, 8, 1);
        #endif
        display.drawBitmap(112, 0, LOGO_battery3, 16, 8, 1);
        display.setCursor(48,0);
        display.setTextColor(WHITE, BLACK);
        switch (State){
            case 1: display.print("Ready."); break;
            case 2: display.print("LP Mod"); break;
            case 3: display.print("SafeMd"); break;
            case 4: display.print("Opened"); break;
            default: PRINT("OLED: Undefine State display")
        }
        
        display.display();
        WiFiS_C = WiFiS;
        FingerS_C = FingerS >= 1;
        ReaderS_C = ReaderS >= 1;
        KeyS_C = KeyS >= 1;
        State_C = State;
    }
    if ((OLEDS == 1) && OLEDStr_C.compareTo(OLEDStr)){
        display.setTextSize(1);
        display.setTextColor(WHITE, BLACK);
        display.setCursor(0, 15);
        display.print("                ");
        display.setCursor(0, 15);
        display.print(OLEDStr.substring(0,15));
        display.display();
        OLED_Fade(3000);
        PRINT("OLED Display: ")
        PRINT(OLEDStr)
        OLEDStr_C = OLEDStr;
        
//        if ((OLEDStr.compareTo("Invalid Card")) && millis() - StrDur > 3000){
//            OLEDStr = "";
//        }
        
    }
    if ((StrFade != -1) && (millis() - StrTime > StrFade)){
        StrFade = -1;
        OLEDStr = "";
    }

}

/******************
 * Function: EEPROM_Read 
 * Discription:
 * Input: void
 * Output: void
 * Global Variables:  
 * 
 ******************/
void EEPROM_Read(int Addr, int len, byte Re[]){
//    byte *Re;
    int j;
//    Re = NULL;
    if ((len > 0) && (Addr + len < EEPROM_CAPACITY)){
//        Re = (byte *)malloc(sizeof(byte) * len);
        j = 0;
        for(int i = Addr; i < Addr + len;  i++)
            Re[j++] = EEPROM.read(i);
    }
//    return Re;
}

void EEPROM_Write(int Addr,int len, byte Content[]){
//    byte *p;
//    p = (byte *)Content;
//    memcpy(p, Content, len);
    PRINT("Write EEPROM")
    if ((len > 0) && (Addr + len < EEPROM_CAPACITY)){
        for (int i = 0; i < len; i++){
            EEPROM.write(Addr + i, Content[i]);
//            PRINT(Content[i])
        }
    }
    EEPROM.commit();
    PRINT("Done")
}
/******************
 * Function: Task_WiFi
 * Discription:
 * Input: void
 * Output: void
 * Global Variables:  
 * States:
   * 0: Disable
   * 1: Connected to ssid
   * 2: 
   * 3: 
   * 4: Failed to connect to AP
 *******************/
 #if WIFI_EN
 void WiFi_Publish(CIDbyte Card0, byte Info){
    static byte PubNum = 1; // timestamp
    byte tmp,sum = 0;
    CIDbyte Tmp, Tmp0 = {0};
    tic();
    PubPayload.Content = "";
    des.encrypt(Tmp.Byte, Card0.Byte, DESkey);
    PRINT("WiFi Publish Payload:")
    for (int i = 0; i < 8; i++){
        tmp = Card0.Byte[i];
        PubPayload.Content += ASCII[tmp >> 4];
        PubPayload.Content += ASCII[tmp & 0x0F];
        PubPayload.p[i] = Tmp.Byte[i];
        sum += Tmp.Byte[i];
//        PRINT(ASCII[Tmp.Byte[i] >> 4])
//        PRINT(ASCII[Tmp.Byte[i] & 0x0F])
    }
    Tmp0.Byte[0] = Info;
    Tmp0.Byte[1] = PubNum;
    sum += Info + PubNum;
    Tmp0.Byte[7] = sum;
    PubPayload.Content += "_";
    PubPayload.Content += ASCII[Info >> 4];
    PubPayload.Content += ASCII[Info & 0x0F];
//    PubPayload.Content += "_";
    PubPayload.Content += ASCII[PubNum >> 4];
    PubPayload.Content += ASCII[PubNum & 0x0F];
    des.encrypt(Tmp.Byte, Tmp0.Byte, DESkey);
    for (int i = 0; i < 8; i++){
        PubPayload.p[i] = Tmp0.Byte[i];
//        PRINT(ASCII[Tmp0.Byte[i] >> 4])
//        PRINT(ASCII[Tmp0.Byte[i] & 0x0F])
    }
    PRINT(".")
    PRINT(PubPayload.Content)
    PRINT(" CheckSum:")
    PRINT(sum)
    PubNum++;
    if (WiFiS >= 3) WiFiS = 2;
    PRINT("Duration")
    PRINT(toc())
 }
 void Task_WiFi(){
    
    const char* ssid = "ASUS";//"IssacRunmin"; //"TP-LINK_1AF3";   // Router name
    const char* password = ""; //"asdfghjkl"; // Router password
    const char* mqtt_server = "192.168.1.2";//"192.168.43.137";//Server address
    const int port=1883;                // MQTT Server port
    static char InTopic[16];
    static char PubTopic[20];
    static char clientName[20];
    static long lastRecAttempt = 0;
    
    static int WiFi_Cycle = 5;
    static int FCount = WiFi_Cycle;
    if (State == 0){ // initialize
        int FailedCount,i;
        WiFi.begin();
        WiFi.begin(ssid,password);
        PRINT("Connecting to ")
        PRINT(ssid)
        for(FailedCount = 0; (FailedCount < 8) &&
        (WiFi.status() != WL_CONNECTED); FailedCount++){
            delay(500);
            PRINT("...")
        }
        if (FailedCount < 8){
            WiFiS = 1;
            PRINT("Success!")
            
        }
        else{
//            WiFiS = 4;
            PRINT("Cannot connect to")
            PRINT(ssid)
        }
        client.setServer(mqtt_server, port);
        client.setCallback(callback);
        InTopic[0] = '\0';
        strcat(InTopic, "WiEye");
        strcat(InTopic, DEVICE_STR);
        InTopic[13] = '\0';
        PRINT(InTopic);
        FCount = WiFi_Cycle;
    }
    else if (State == 1){ // normal state
        switch(WiFiS){
            case 0: // no connection
                if (FCount-- <= 0){
                    FCount = WiFi_Cycle;
                    if (WiFi.status() == WL_CONNECTED){
                        PRINT("WiFi #0: Reconnect!")
                        WiFiS++;
                    }
                }
                break;
            case 1: // connect to broker
                if (FCount-- <= 0){
                    FCount = WiFi_Cycle;
                    PRINT("Check Sever");
                    if (!client.connected()){
                        String Random;
                        Random = String(random(99999));
                        clientName[0] = '\0';
                        strcpy(clientName, Random.c_str());
                        strcat(clientName, "WiEye");
                        strcat(clientName, DEVICE_STR);
                        clientName[18] = '\0';
                        PRINT(clientName);
                        InTopic[0] = '\0';
                        strcat(InTopic, "WiEye");
                        strcat(InTopic, DEVICE_STR);
                        InTopic[13] = '\0';
                        PRINT(InTopic);
                        if (client.connect(clientName)){
                            PRINT("WiFi #1:")
                            client.publish(LOG_TOPIC, DEVICE_STR);
                            client.subscribe(InTopic);
                            PubTopic[0] = '\0'; 
                            strcat(PubTopic, InTopic);
                            strcat(PubTopic, "_Out");
                            PubTopic[17] = '\0';
                            PRINT("Connect to client!")
                            PRINT("Ori Topic: ")
                            PRINT(LOG_TOPIC)
                            PRINT("Public Topic: ")
                            PRINT(PubTopic)
                            PRINT("InTopic: ")
                            PRINT(InTopic)
                            WiFiS = 3;
                        }
                        else
                        PRINT("Connect to Sever failed!");
                    }
                    else{
                        WiFiS = 3;
                        PRINT("WiFi #1: Already Connected!")
                    }
                }
//                else PRINT(FCount);
                break;
            case 2: // may use for publish 
                #if WIFI_ENCRYPT
                client.publish(PubTopic, PubPayload.p, 16);
                #else
                char tmp[100];
                strcpy(tmp, PubPayload.Content.c_str());
                PRINT("Publish: ");
                PRINT(tmp);
                delay(1000);
                client.publish(PubTopic, tmp);
                #endif
                WiFiS++;
//            break;
            case 3: 
                client.loop();
            break;
            case 5: // Wait for main_task;
                if (FCount-- <= 0){
                    FCount = WiFi_Cycle;
                    WiFiS = 3;
                    PRINT("WiFi #5: TimeOut!");
                }
            break;
            default: PRINT("Unknown WiFiState #")
                PRINT(WiFiS)
        }
    }
}
#else
void Task_WiFi(){ if (State == 0) PRINT("Wi-Fi Disabled!")}
void WiFi_Publish(CIDbyte Card0, byte Info){ PRINT("No WIFI")}
#endif

/******************
 * Function: Task_Pad()
 * Discription: scan and detect the key, including key down,
 *    up, and finally get the Key, add 1 to KeyNum
 * Input: void
 * Output: void
 * Global variables: Key, KeyNum, KeyS
 ******************/
#if KEY_EN
void Task_Pad(){
    static byte KeyC = 1;
    static byte KEY_CYCLE = 4;
    static int key = 0;
    static bool down = false;
    static String PWSig = "";
    static uint32 duration = 0;
    byte Err;
    if (State == 0){ // Init
        Wire.beginTransmission(Keypad_DEVID);
        Err = Wire.endTransmission();
        if (Err == 0){
            KeyS = 1;
            CapaTouch.begin();
            PRINT("Keypad I2C: Find")
        }
        else if (Err == 4)
                PRINT("Keypad I2C: Unknow Error")
            else
                PRINT("Keypad I2C: No Connection")
    }
    else{ // State == 1 or 3
        switch (KeyS){
          
          case 1: key = 0; KeyC = 1; PW = 0; PWSig = ""; down = false; KeyS++; 
              PRINT("Keypad Init")
              break;
          case 2:
              if (KeyC == 1){
                  key = CapaTouch.keyPad();
                  if (!down && key > 0){
                      down = true;
                      duration = millis();
                      Key = key;
                  }
                  if (down && key == -1){
                      down = false;
                      duration = millis() - duration;
                      #if DEBUG
                          Serial.print("key: ");
                          Serial.println(Key);
                      #endif
                      if (duration > KEY_DUR){
//                          PRINT("valid key press")
                          
                          switch (Key){
                          
                          case 9: break;
                          case 11: PW = 0; PWSig = ""; break;
                          case 10: if (PW < 100000000) {
                                PW = PW * 10; PWSig += '*';
                              }
                            break;
                          default: 
                              if (PW < 100000000) {
                                PW = PW * 10 + Key; PWSig += '*';
                              }
                          }
                          if (Key == 9)
                              Confirm = true;
                          else{
                              Confirm = false;
                              if (State == 1)
                                  OLEDStr = PWSig;
                              else if (State == 3){
                                  PW = Key;
                                  OLEDStr = ASCII[Key];
                              }
                          }
                          KeyNum++;
                      }
                      else
                          PRINT("invalid key press")
                  }
              }
              KeyC++;
              if (KeyC > KEY_CYCLE)
                  KeyC = 1;
          break;
          default: PRINT("Unknown Key State!"); KeyS = 1;
        }
        
    }
}
#else
void Task_Pad(){if (State == 0) PRINT("Keypad Disabled!")}
#endif


/******************
 * Function: Task_WiFi
 * Discription:
 * Input: void
 * Output: void
 * Global Variables:  
 * States: ReaderS
   * 0: Disable
   * 1: Connected to ssid
   * 2: 
   * 3: 
   * 4: Failed to connect to AP
 *******************/
#if READER_EN
void Task_Reader(){
    const byte StoreVid = 0x92;
    const byte ReadCount = 5; // an odd number
    const byte trailerBlock   = 7; // 验证密码的数据块
    static MFRC522::MIFARE_Key key;            // KEY
    static MFRC522::StatusCode status;         // Status
    static byte buffer[18];                    // MFRC522 Buffer
    static byte size = sizeof(buffer);
    static CIDbyte IDSeries[ReadCount];
    static int ReaderC = 1; 
    static int Reader_CYCLE = 1; 
    static int FailedCount = 3;
    static int Idx;
    if (State == 0){
        byte ver;
        tic();
        for (byte i = 0; i < 6; i++) {
            key.keyByte[i] = 0xFF;
        }
        SPI.begin();
        Reader.PCD_Init();
        ver = Reader.PCD_ReadRegister(MFRC522::VersionReg);
        PRINT("Reader #0: Firmware Version: ")
        PRINT(ver)
        ReaderS = 1;
        switch (ver) {
            case 0x00:
            case 0xFF:
                PRINT("Reader #0: Communication failure!")
                ReaderS = 0;
                break;
            case 0x88: PRINT("Version: (clone)"); break;
            case 0x90: PRINT("Version: v0.0"); break;
            case 0x91: PRINT("Version: v1.0"); break;
            case 0x92: PRINT("Version: v2.0"); break;
            default: PRINT("Version: (unknown)");
        }
        
    }
    else{ // State = 1 or 3
        switch (ReaderS){
            case 0: 
                if (ReaderC-- <= 0){
                    ReaderC = Reader_CYCLE;
                    if (StoreVid == Reader.PCD_ReadRegister
                        (MFRC522::VersionReg)) {
                        ReaderS++;
                        PRINT("Reader Reconnect!")
                    }
                }
                break;
            case 1: // Wait if in safe mode
                if (State != 3) ReaderS++;
                else break;
            case 2: 
                if (ReaderC-- <= 0){
                    ReaderC = Reader_CYCLE;
                    if (Reader.PICC_IsNewCardPresent() && 
                        Reader.PICC_ReadCardSerial()){
                        for (int i = 0; i < Reader.uid.size; i++)
                            Card.Byte[i] = Reader.uid.uidByte[i];
                        ReaderS = 4;
                        PRINT("Reader #2: New Card!")
                    }
                }
                if (ReaderS != 4)
                break;
            case 3:
//                tic();
//                if (!Reader.PICC_IsNewCardPresent()){
//                    PRINT("Reader #3: Failed once")
//                    FailedCount--;
//                    if (FailedCount <= 0){
//                        FailedCount = 3;
//                        PRINT("ERROR Read Card ID")
//                        ReaderS = 1;
//                    }
//                }
//                else {
//                    FailedCount = 3;
//                    for (int i = 0; i < Reader.uid.size; i++){
//                        IDSeries[Idx].Byte[i] = Reader.uid.uidByte[i];
//                    }
//                    #if DEBUG
//                    Serial.print("  Find");
//                    Serial.println(Idx);
//                        for (int i = 0; i < Reader.uid.size; i++)
//                            Serial.print(IDSeries[Idx].Byte[i],HEX);
//                    Serial.println();
//                    #endif
//                    Idx++;
//                    if (Idx == ReadCount){
//                        ReaderS++;
//                        break;
//                    }
//                }
//                
//                PRINT("Reader #3: duration");
//                PRINT(toc())
//                if (ReaderS != 4)
                ReaderS++;
                break;
            case 4:
                byte Counts[ReadCount];
                bool Success;
                int i,j;
                MFRC522::PICC_Type piccType;
                Success = false;
                tic();
                PRINT("Reader #4:")
//                for (i = 1; i < ReadCount; i++){
//                    for (int j = 0; j < i; j++)
//                        if (IDSeries[i].Int == IDSeries[j].Int){
//                            Counts[j]++;
//                            if (Counts[j] > ReadCount/2)
//                                Success = true;
//                            break;
//                        }
//                    if (Success) break;
//                }
//                Card.Int = IDSeries[j].Int; // 卡信息存储在此
//                if (!Success){ // Failed to find card
//                    PRINT("Failed to find Card")
//                    ReaderS = 1;
//                    break;
//                }
                // sucessfully select a card
                #if DEBUG
                Serial.print("Find Card:");
                for (int i = 0; i < 8; i++)
                    Serial.print(Card.Byte[i],HEX);
                Serial.println();
                #endif
                piccType = Reader.PICC_GetType(Reader.uid.sak);
                if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
                    &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
                    &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K){
                    PRINT("Unsuported Card tpye(No MIFARE Classic)")
                    ReaderS = 1;
                    OLEDStr = "Card type Err";
                    Reader.PICC_HaltA(); 
                    break;
                }
                
                #if CARD_AUTH
                status = (MFRC522::StatusCode) Reader.PCD_Authenticate
                    (MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock,
                    &key, &(Reader.uid));
                if (status != MFRC522::STATUS_OK){
                    PRINT("Authenticate card failed!")
                    Reader.PICC_HaltA(); 
                    ReaderS = 1;
                    break;
                }
                status = (MFRC522::StatusCode) Reader.MIFARE_Read
                    (trailerBlock, buffer, &size);
                if (status != MFRC522::STATUS_OK){
                    PRINT("Read card failed!")
                    Reader.PICC_HaltA(); 
                    ReaderS = 1;
                    break;
                }
                for (i = 0; i < 4; i++)
                    if (AuthCard.Byte[i] != buffer[i]) break;
                if ( i == 4 ){
                    PRINT("Card Authenticated!")
                    ReaderS++;
                    if (State != 3)
                        Reader.PICC_HaltA();
                }
                else{
                    PRINT("Block not equal!")
                    ReaderS = 1;
                    Reader.PICC_HaltA();
                }
                #else
                ReaderS++;
                Reader.PICC_HaltA();
                if (State == 3) 
                    ReaderS=6;
                #endif
                PRINT("duration:")
                PRINT(toc())
                break;
            case 5: // Wait for respones;
                if (FailedCount > 0) FailedCount--;
                else{
                    FailedCount = 3;
                    ReaderS = 1;
                    PRINT("Reader #5: TimeOut")
                }
                break;
            case 6: // Safemode for initial card
//                Reader.PICC_HaltA();
            break;
            default: 
                ReaderS = 0;
                PRINT("Reader #?: Unknown State ")
        }
        
    }
}
#else
void Task_Reader(){if (State == 0) PRINT("RFID READER Disabled!")}
#endif



/******************
 * Function: Task_Finger
 * Discription:
 * Input: void
 * Output: void
 * Global Variables:  
 * State: 
 * 
 ******************/
 

#if FINGER_EN
void Finger_Check_Comm(uint8_t p){
    static int FCount = 3;
    if (p == FINGERPRINT_PACKETRECIEVEERR){
        PRINT("Finger #c: Communication error")
        if (FCount > 0) FCount--;
        else{
            FCount = 3;
            if (!finger.verifyPassword()){
                FingerS = 0;
                PRINT("Finger #c: Disconnected")
            }
        }
    }
    else FCount = 3;
}
void Task_Finger(){
    static int FingerC = 1;
    static int Finger_CYCLE = 10;
    static int FailedCount = 3;

    uint8_t p;
    if (State == 0){
        int FailedCount,i;
        pinMode(PIN_RX, INPUT);
        pinMode(PIN_TX, OUTPUT);
        finger.begin(57600);
        
        for (FailedCount = 0; FailedCount < 6 &&
        !finger.verifyPassword(); FailedCount++){
            delay(100);
            PRINT("No FingerModel")
        }
            
        if (FailedCount < 6) {
            FingerS = 1;
            PRINT("Found fingerprint sensor!")
        }
        else
            PRINT("Did not find fingerprint sensor :(")
        FailedCount = 3;
    }
    else if (State == 1){ // Normal State
        
        switch (FingerS){
            case 0: if (FingerC-- <= 0){
                FingerC = Finger_CYCLE;
//                p = finger.getImage();
//                if (p != FINGERPRINT_PACKETRECIEVEERR){
                if (finger.verifyPassword()) {
                    FingerS = 1;
                    PRINT("FingerSensor Reconnect!")
                }
            }
            break;
            case 1:if (FingerC-- <= 0){
                tic();
//                PRINT("Finger #1: ")
                p = finger.getImage();
                FingerC = Finger_CYCLE;
                Finger_Check_Comm(p);
                if (p == FINGERPRINT_OK){
                    FingerS++;
                    PRINT("Image Taken")
                }
                else if (p != FINGERPRINT_NOFINGER && 
                    p != FINGERPRINT_PACKETRECIEVEERR)
                    PRINT("Unknown error")
//                PRINT("duration")
//                PRINT(toc())
//                if (FingerS != 2) // Commit this code if 
                break; 
            }
            else 
                break;
            case 2:
                tic();
                PRINT("Finger #2: ")
                p = finger.image2Tz();
                Finger_Check_Comm(p);
                if (p == FINGERPRINT_OK){
                    FingerS++;
                    PRINT("Image converted")
                }
                else{
                    switch (p){
                        case FINGERPRINT_IMAGEMESS: 
                          PRINT(" Image too messy");break;
                        case FINGERPRINT_FEATUREFAIL:
                        case FINGERPRINT_INVALIDIMAGE:
                          PRINT("Could not find fingerprint features");
                          break;
                        default: PRINT("Other error..")
                    }
                    PRINT("Restart")
                    FingerS = 1;
                }
                PRINT("duration")
                PRINT(toc())
                if (FingerS != 3)  // Commit this code if time is not engouth 
                break; 
            case 3:
                tic();
                PRINT("Finger #3: ")
                p = finger.fingerFastSearch();
                Finger_Check_Comm(p);
                if (p == FINGERPRINT_OK){
                    if (finger.confidence < Finger_Threshold)
                      FingerS = 5;
                    else
                      FingerS++;
                    PRINT("Found a print match!ID:")
                    PRINT(finger.fingerID)
                    PRINT("Confidence: ")
                    PRINT(finger.confidence)
                }
                else if (p == FINGERPRINT_NOTFOUND){
                    FingerS = 5;
                    PRINT("Did not find a print match!")
                }
                else{
                    PRINT("Unknown error.. Restart")
                    FingerS = 1;
                }
                PRINT("duration")
                PRINT(toc())
                break;
            case 4:
                if (FailedCount > 0) FailedCount--;
                else{
                    FailedCount = 3;
                    FingerS = 1;
                    PRINT("Finger #4: TimeOut")
                }
                break;
            case 5:  // Finger not match
                if (FailedCount > 0) FailedCount--;
                else{
                    FailedCount = 3;
                    FingerS = 1;
                    PRINT("Finger #5: TimeOut")
                }
                break;
            default: 
                FingerS = 0;
                PRINT("Finger #?: Unknown State ")
        }
    }
    else if (State == 3){ // Safe mode
        switch (FingerS){
            case 0: if (FingerC-- <= 0){
                FingerC = Finger_CYCLE;
                if (finger.verifyPassword()) {
                    FingerS = 1;
                    PRINT("FingerSensor Reconnect!")
                }
                break;
            }
            case 1: 
//                if (Finger_Idx == 0)
                  for (Finger_Idx = 9; Finger_Idx < 128; Finger_Idx++)
                    if (EEPROM.read(Finger_Idx) == 0) break;
                Finger_Idx -= 8;
                PRINT("Finger #3.1: Find avaliable id")
                PRINT(Finger_Idx)
                FingerS++;
            break;
            case 2: // Wait for Enroll
                
            break;
            case 3:
                tic();
                p = finger.getImage();
                Finger_Check_Comm(p);
                if (p == FINGERPRINT_OK){
                    FingerS++;
                    PRINT("Finger #3.3: Image Taken")
                    PRINT("duration")
                    PRINT(toc())
                }
                else if (p != FINGERPRINT_NOFINGER)
                    PRINT("Finger #3.3: Unknown error")
                if (FingerS == 4) Task_Finger();
                break;
            case 4:
                tic();
                PRINT("Finger #3.4: ")
                p = finger.image2Tz(1);
                Finger_Check_Comm(p);
                if (p == FINGERPRINT_OK){
                    FingerS++;
                    OLEDStr = "Please Remove Finger...";
                    PRINT("Image converted")
                    PRINT("Remove Finger...")
                }
                else{
                    switch (p){
                        case FINGERPRINT_IMAGEMESS: 
                          PRINT("Image too messy");break;
                        case FINGERPRINT_FEATUREFAIL:
                        case FINGERPRINT_INVALIDIMAGE:
                          PRINT("Could not find fingerprint features");
                          break;
                        default: PRINT("Other error..")
                    }
                    PRINT("Restart")
                    FingerS = 3;
                }
                PRINT("duration")
                PRINT(toc())
                break;
            case 5: // Remove finger
                if (FailedCount > 0) FailedCount--;
                else{
                    FailedCount = 3;
                    p = finger.getImage();
                    Finger_Check_Comm(p);
                    if (p == FINGERPRINT_NOFINGER){
                        FingerS++;
                        OLEDStr = "Place Finger Again!";
                        PRINT("Finger #3.5: Finger Removed")
                    } 
                }
                if (FingerS != 6)
                break;
            case 6: 
                p = finger.getImage();
                Finger_Check_Comm(p);
                if (p == FINGERPRINT_OK){
                    FingerS++;
                    PRINT("Finger #3.6: Image Taken")
                }
                else if (p != FINGERPRINT_NOFINGER)
                    PRINT("Finger #3.6: Unknown error")
                if (FingerS != 7)
                    break;
            case 7: 
                PRINT("Finger #3.7: ")
                p = finger.image2Tz(2);
                Finger_Check_Comm(p);
                if (p == FINGERPRINT_OK){
                    FingerS++;
                    PRINT("Image converted")
                }
                else{
                    switch (p){
                        case FINGERPRINT_IMAGEMESS: 
                          PRINT("Image too messy");break;
                        case FINGERPRINT_FEATUREFAIL:
                        case FINGERPRINT_INVALIDIMAGE:
                          PRINT("Could not find fingerprint features");
                          break;
                        default: PRINT("Other error..")
                    }
                    PRINT("Restart")
                    OLEDStr = "Try Again";
                    FingerS = 3;
                }
                if (FingerS != 8)
                    break;
            case 8:
                tic();
                PRINT("Finger #3.8: ")
                p = finger.createModel();
                Finger_Check_Comm(p);
                if (p == FINGERPRINT_OK){
                    FingerS++;
                    PRINT("Prints matched!")
                }
                else {
                    FingerS = 3;
                    if (p == FINGERPRINT_ENROLLMISMATCH)
                        PRINT("Fingerprints did not match!")
                    else
                        PRINT("Unknown error")
                    OLEDStr = "Not Match, Try Again";
                }
                PRINT("duration")
                PRINT(toc())
                if (FingerS != 9)
                    break;
            case 9:
                tic();
                PRINT("Finger #3.9: ")
                p = finger.storeModel(Finger_Idx);
                Finger_Check_Comm(p);
                if (p == FINGERPRINT_OK){
                    FingerS++;
                    FailedCount = 999;
                    PRINT("Prints stored!")
                    OLEDStr = "Success";
                }
                else{
                    switch(p){
                        case FINGERPRINT_BADLOCATION:
                            PRINT("Could not store in that location")
                            break;
                        case FINGERPRINT_FLASHERR:
                            PRINT("Error writing to flash")
                            break;
                        default: PRINT("Unknown error")
                    }
                    FingerS = 3;
                }
                PRINT("duration");
                PRINT(toc());
                break;
            case 10:
            case 11: // Waiting for write into EEPROM
                if (FailedCount > 0) FailedCount--;
                else{
                    FailedCount = 3;
                    FingerS = 2;
                    Finger_Idx = 0;
                    PRINT("Finger #3.10: TimeOut")
                }
                break;
            default: 
                FingerS = 0;
                PRINT("Finger #?: Unknown State ")
        }
    }
}
#else
void Task_Finger(){ if (State == 0) {FingerS = 0;PRINT("Fingerprint Disabled!")}}
#endif



/******************
 * Function: Task_Main 
 * Discription:
 * Input: void
 * Output: void
 * Global Variables:  
 * State: 
 * 
 ******************/
 void PrintSafeOpt(){
    if (State == 3){
        OLEDStr = "";
        #if READER_EN
        OLEDStr += "1-C ";
        #endif
        #if FINGER_EN
        OLEDStr += "2-F ";
        #endif
    //            #if FINGER_EN && READER_EN
    //            OLEDStr += "3-M ";
    //            #endif
        OLEDStr += "4-Ex";
    //            OLEDStr = "1-Card 2-Fin 3-Match";
        PRINT(OLEDStr)
    }
 }
void Task_Main(){
    static uint32 PW0 = 0;
    static uint32 KeyNum0 = 0;                 
    static byte NoAct;
    static bool SafeWorking = false;
    static uint32 OpenDur = 0;
    int State_copy;
    IDbyte tmp;
    
    NoAct = 0;
    State_copy = State;
    if (State == 1){ // Normal
        #if KEY_EN
        if (KeyNum0 != KeyNum){
            KeyNum0 = KeyNum;
            if (Confirm){
                Confirm = false;
                if (PW == DEVPW.Int){
                    State = 3; // Safe mode
                }
                else if (PW_TmpI > 0 && PW_Tmp == PW){
                        CIDbyte CardTmp = {0};
                        PW_TmpI--;
                        State = 4; // Door open
                        WiFi_Publish(CardTmp, 0x30);
                    }
            }
        }
        else NoAct |= 0x01;
        #endif
        #if FINGER_EN
        if (FingerS == 4){
            byte Idx;
            Idx = EEPROM.read(finger.fingerID + 8);
            if (Idx == 0){
                OLEDStr = "Ivaliad finger ID";
                OLED_Fade(3000);
                PRINT("Detect Finger but did not match ID card...")
                FingerS = 1;
            }
            else{ // successfully authenticed
                State = 4;
                EEPROM_Read((int)(128 + Idx * 16), 8, CardAuth.Byte);
                WiFi_Publish(CardAuth, 0x20); // FingerPrint Auth
//                for (int i = 0; i < 8; i++)
//                    CardAuth.Byte[i] = CardID[i];
            }
        }
        else if (FingerS == 5){
            OLEDStr = "Invalid Finger. Try Again";
            OLED_Fade(3000);
            PRINT("Model did not match any finger")
            FingerS = 1;
        }
        #endif
        #if READER_EN
        if (ReaderS == 5){
            CIDbyte CardTmp;
            CIDbyte Card_Info;
            int i,j;
//            for (i = 0; i < 8; i++)
//                PRINT(Card.Byte[i]);
            for (i = 1; i <= UserCount; i++){
                EEPROM_Read((int)(128 + i * 16), 8, CardTmp.Byte);
                EEPROM_Read((int)(128 + i * 16 + 8), 2, Card_Info.Byte);
                if (CardTmp.Int == Card.Int || ((Card_Info.Byte[0] & 0x01) == 0))
                    break;
            }
            if (i <= UserCount){
                State = 4;
                CardAuth.Int = Card.Int;
                WiFi_Publish(CardAuth, 0x10);
            }
            else{
                OLEDStr = "Invalid Card";
                OLED_Fade(3000);
                ReaderS = 2;
            }
        }
        #endif
        #if WIFI_EN
//        PRINT(WiFiS);
        if (WiFiS == 5){
            State = 4;
            WiFi_Publish(CardAuth, 0x40);
        }
        #endif
        #if OLED_EN
        OLED_Display_Mode();
        #endif
        
    }
    else if (State == 4){ // Door Open
//        CardAuth.Int = 0;
        
        if ((millis() - OpenDur) > OPEN_DUR){
            State = 1;
        }
    }
    else if (State == 3){ // Safe Mode, not define
        #if KEY_EN
        if (KeyNum0 != KeyNum){
            KeyNum0 = KeyNum;
            if (Confirm){
                PRINT("Confirm-Main3")
                Confirm = false;
                if (!SafeWorking){
                    SafeWorking = true;
                    switch (PW){
                       case 1:
                          if (ReaderS == 1){
                              ReaderS++;
                              OLEDStr = "Put new card";
                              PRINT("Reader Start working")
                          }
                          else{
                             OLEDStr = "Unknown ReaderS";
                             PRINT("Unknown Reader State #")
                             PRINT(ReaderS)
                          }
                      break;
                      case 2: 
                          if (FingerS == 2){
                              FingerS++;
                              OLEDStr = "Put new finger";
                              PRINT("Finger Start working")
                          }
                          else{
                             OLEDStr = "Unknown FingerS";
                             PRINT("Unknown Finger State #")
                             PRINT(FingerS)
                          }
                      break;
                      case 4: 
                          State = 1;
                      break;
                      case 5: 
                          if (ReaderS > 1) ReaderS = 1;
                          if (FingerS > 2) FingerS = 2;
                          SafeWorking = false;
                          PrintSafeOpt();
                      break;
                      default: SafeWorking = false;PRINT("Unknown Key#") PRINT(PW)
                    }
                }
                else PRINT("Something IS WORKING")

            }
        }
        #endif
        #if READER_EN
        if (ReaderS == 5){
            CIDbyte CardTmp;
            CIDbyte Card_Info;
            byte i;
            ReaderS = 1;
            SafeWorking = false;
            for (i = 1; i <= UserCount; i++){
                EEPROM_Read((int)(128 + i * 16), 8, CardTmp.Byte);
                EEPROM_Read((int)(128 + i * 16 + 8), 2, Card_Info.Byte);
                if (CardTmp.Int == Card.Int || ((Card_Info.Byte[0] & 0x01) == 0))
                    break;
            }
            if (CardTmp.Int == Card.Int){
                OLEDStr = "Card already in";
                PRINT("Card already in the EEPROM")
            }
            else
                EEPROM_Write((int)(128 + i * 16), 8, Card.Byte);
            if (i > UserCount){
                byte check_sum; 
                UserCount++;
                EEPROM.write(4, UserCount);
                check_sum = EEPROM.read(7);
                check_sum++;
                EEPROM.write(7, check_sum);
                EEPROM.commit();
                PRINT("New Card recorded!")
                OLEDStr = "Done";
                OLED_Fade(3000);
            }
            if (FingerS == 11){
                EEPROM.write(8+Finger_Idx, i);
                EEPROM.commit();
                PRINT("Finger print Matched! #")
                PRINT(Finger_Idx);
                PRINT(i);
                if (ReaderS > 1) ReaderS = 1;
                if (FingerS > 2) FingerS = 2;
                SafeWorking = false;
                OLEDStr = "Done";
                
            }
            
        }
        #endif 
        #if FINGER_EN
        if (FingerS == 10){
            FingerS++;
            if (ReaderS == 1){
                OLEDStr = "Put Card to match";
                ReaderS++;
                PRINT("Put the card to match finger")
            }
            else {
                PRINT("Unknown Reader State")
                PRINT(ReaderS)
                OLEDStr = "Failed Card";
                PrintSafeOpt();
                if (ReaderS > 1) ReaderS = 1;
                if (FingerS > 2) FingerS = 2;
                SafeWorking = false;
                
            }
            
        }
        
        #endif
        if(!SafeWorking && OLEDStr.compareTo(""))
            PrintSafeOpt();
    }
    if (State_copy != State){ // State change
        PRINT("State Change! State #")
        PRINT(State)
        KeyS = 1;
        if (FingerS > 2)
            FingerS = 2;
        else
          FingerS = 0;
        ReaderS = 0;
        OLEDStr = "";
        if (State == 3){
            SafeWorking = false;
            PrintSafeOpt();
        }
        else if (State == 4){
            OpenDur = millis();
            if (OLEDS == 1){
              display.clearDisplay();
              display.drawBitmap(0, 16, LOGO_closed, 128, 48, 1);
              display.display();
            }
            digitalWrite(PIN_RELAY, HIGH);
        }
        else if(State == 1){
            if (OLEDS == 1){
                display.clearDisplay();
                display.drawBitmap(0, 16, LOGO_main, 128, 48, 1);
                display.display();
            }
            digitalWrite(PIN_RELAY, LOW);
        }
        
    }
        
}

void loop() {
    static uint LSM = 0;
    uint32 St, Ed;
    St = micros();
    Task_Pad();
    switch (LSM){
        case 0: Task_Reader(); break;
        case 1: Task_Finger(); break;
        case 2: Task_WiFi(); break;
        case 3: Task_Reader(); break;
        case 4: Task_Finger(); break;
//        case 6: Task_Finger(); break;
        case CYCLE: Task_Main(); OLED_Display_Mode(); break;
    }
    LSM++;
    if (LSM > CYCLE) LSM = 0;
    Ed = micros();
//    PRINT(LSM)
    if (Ed < St) delay(INTERVAL); // Overflow over 70min
    else if ((Ed - St) / 1000 < INTERVAL)
        delay(INTERVAL - (Ed - St) / 1000);
}
