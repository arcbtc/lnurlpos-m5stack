
#include <M5Stack.h>
#define KEYBOARD_I2C_ADDR     0X08
#define KEYBOARD_INT          5
#include "SPIFFS.h"
#include "FS.h"

#include <string.h>
#include "qrcode.h"
#include "Bitcoin.h"
#include <Hash.h>
#include <Conversion.h>
#include <WiFi.h>
#include "esp_adc_cal.h"

////////////////////////////////////////////////////////
////////CHANGE! USE LNURLPoS EXTENSION IN LNBITS////////
////////////////////////////////////////////////////////

String baseURL = "https://legend.lnbits.com/lnurlpos/api/v1/lnurl/UZsLkBSzdDqEFgc3RAs8rj";
String key = "UzhUjUGFvEtJRaVSpxxNCa";
String currency = "USD";

//////////////SLEEP SETTINGS///////////////////
bool isSleepEnabled = true;
int sleepTimer = 30; // Time in seconds before the device goes to sleep

//////////////VARIABLES///////////////////
String dataId = "";
bool paid = false;
bool shouldSaveConfig = false;
bool down = false;
const char *spiffcontent = "";
String spiffing;
String lnurl;
String choice;
String payhash;
String key_val;
String cntr = "0";
String inputs;
int keysdec;
int keyssdec;
float temp;
String fiat;
float satoshis;
String nosats;
float conversion;
String virtkey;
String payreq;
int randomPin;
bool settle = false;
String preparedURL;
RTC_DATA_ATTR int bootCount = 0;
long timeOfLastInteraction = millis();
bool isPretendSleeping = false;

#include "MyFont.h"

#define BIGFONT &FreeMonoBold24pt7b
#define MIDBIGFONT &FreeMonoBold18pt7b
#define MIDFONT &FreeMonoBold12pt7b
#define SMALLFONT &FreeMonoBold9pt7b
#define TINYFONT &TomThumb

SHA256 h;

//////////////KEYPAD///////////////////

int checker = 0;
char maxdig[20];

//////////////MAIN///////////////////

void setup(void) {
  M5.begin();
  Wire.begin();
  pinMode(4,OUTPUT); 
  digitalWrite(4,HIGH);
  btStop();
  WiFi.mode(WIFI_OFF);
  h.begin();

  if(bootCount == 0)
  {
    logo();
    delay(3000);
  }
  else
  {
    wakeAnimation();
  }
  ++bootCount;
  Serial.println("Boot count" + bootCount);
}

void loop() {
  digitalWrite(4,HIGH);
  inputs = "";
  settle = false;
  displaySats();
  bool cntr = false;

  while (cntr != true){
    M5.update();
    get_keypad(); 
    String key = key_val;
    if (key != NULL)
    {
      isPretendSleeping = false;
      timeOfLastInteraction = millis();
      virtkey = String(key);
      if (virtkey == "#"){
        makeLNURL();
        qrShowCode();
        int counta = 0;
        while (settle != true){
          M5.update();
          get_keypad(); 
          String key = key_val;
          virtkey = String(key);
          if (virtkey == "*"){
            timeOfLastInteraction = millis();
            M5.Lcd.fillScreen(TFT_BLACK);
            settle = true;
            cntr = true;
          }
          else if (virtkey == "#"){
            timeOfLastInteraction = millis();
            showPin();
          }
        }
      }
      else if (virtkey == "*")
      {
        M5.Lcd.fillScreen(TFT_BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(TFT_WHITE);
        key_val = "";
        inputs = "";
        nosats = "";
        virtkey = "";
        cntr = "2";
      }
      displaySats();
    }
  }
}

void get_keypad()
{
   if(digitalRead(KEYBOARD_INT) == LOW) {
    Wire.requestFrom(KEYBOARD_I2C_ADDR, 1);  // request 1 byte from keyboard
    while (Wire.available()) { 
       uint8_t key = Wire.read();                  // receive a byte as character
       key_val = key;

       if(key != 0) {
        if(key >= 0x20 && key < 0x7F) { // ASCII String
          if (isdigit((char)key)){
          key_val = ((char)key);
          }
          else {
          key_val = "";
        } 
        }
      }
    }
  }
}

///////////DISPLAY///////////////

void qrShowCode()
{
  M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.qrcode(lnurl,45,0,240,14);
  delay(100);
}

void showPin()
{
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setFreeFont(SMALLFONT);
  M5.Lcd.setCursor(30, 25);
  M5.Lcd.println("PAYMENT PROOF PIN");
  M5.Lcd.setCursor(60, 80);
  M5.Lcd.setTextColor(TFT_PURPLE, TFT_BLACK); 
  M5.Lcd.setFreeFont(BIGFONT);
  M5.Lcd.println(randomPin);
}

void displaySats()
{
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK); // White characters on black background
  M5.Lcd.setFreeFont(MIDFONT);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.println("AMOUNT THEN #");
  M5.Lcd.setCursor(60, 130);
  M5.Lcd.setFreeFont(SMALLFONT);
  M5.Lcd.println("TO RESET PRESS *");

  inputs += virtkey;
  
  M5.Lcd.setFreeFont(MIDFONT);
  M5.Lcd.setCursor(0, 80);
  M5.Lcd.print(String(currency) + ":");
  M5.Lcd.setFreeFont(MIDBIGFONT);
  M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);

  if(currency != "sats")
  {
    float amount = float(inputs.toInt()) / 100;
    M5.Lcd.println(amount);
  }
  else
  {
    int amount = inputs.toInt();
    M5.Lcd.println(amount);
  }
  delay(50);
  virtkey = "";
}

void logo()
{
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK); // White characters on black background
  M5.Lcd.setFreeFont(BIGFONT);
  M5.Lcd.setCursor(7, 70);  // To be compatible with Adafruit_GFX the cursor datum is always bottom left
  M5.Lcd.print("LNURLPoS"); // Using tft.print means text background is NEVER rendered

  M5.Lcd.setTextColor(TFT_PURPLE, TFT_BLACK); // White characters on black background
  M5.Lcd.setFreeFont(SMALLFONT);
  M5.Lcd.setCursor(42, 90);          // To be compatible with Adafruit_GFX the cursor datum is always bottom left
  M5.Lcd.print("Powered by LNbits"); // Using tft.print means text background is NEVER rendered
}

void to_upper(char *arr)
{
  for (size_t i = 0; i < strlen(arr); i++)
  {
    if (arr[i] >= 'a' && arr[i] <= 'z')
    {
      arr[i] = arr[i] - 'a' + 'A';
    }
  }
}


void callback(){
}

/**
 * Awww. Show the go to sleep animation
 */
void sleepAnimation() {
    printSleepAnimationFrame("(o.o)", 500);
    printSleepAnimationFrame("(-.-)", 500);
    printSleepAnimationFrame("(-.-)z", 250);
    printSleepAnimationFrame("(-.-)zz", 250);
    printSleepAnimationFrame("(-.-)zzz", 250);
    digitalWrite(4,LOW);
}

void wakeAnimation() {
    printSleepAnimationFrame("(-.-)", 100);
    printSleepAnimationFrame("(o.o)", 200);
    M5.Lcd.fillScreen(TFT_BLACK);
}

/**
 * Print the line of the animation
 */
void printSleepAnimationFrame(String text, int wait) {
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(5, 80);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK); 
  M5.Lcd.setFreeFont(BIGFONT);
  M5.Lcd.println(text);
  delay(wait);
}

/**
 * Get the voltage going to the device
 */
float getInputVoltage() {
    delay(100);
    uint16_t v1 = analogRead(34);
    return ((float)v1 / 4095.0f) * 2.0f * 3.3f * (1100.0f / 1000.0f);
}

/**
 * Does the device have external or internal power?
 */
bool isPoweredExternally() {
  float inputVoltage = getInputVoltage();
  if(inputVoltage > 4.5)
  {
    return true;
  }
  else
  {
    return false;
  }
  
}

//////////LNURL AND CRYPTO///////////////

void makeLNURL()
{
  randomPin = random(1000, 9999);
  byte nonce[8];
  for (int i = 0; i < 8; i++)
  {
    nonce[i] = random(256);
  }
  byte payload[51]; // 51 bytes is max one can get with xor-encryption
  size_t payload_len = xor_encrypt(payload, sizeof(payload), (uint8_t *)key.c_str(), key.length(), nonce, sizeof(nonce), randomPin, inputs.toInt());
  preparedURL = baseURL + "?p=";
  preparedURL += toBase64(payload, payload_len, BASE64_URLSAFE | BASE64_NOPADDING);
  Serial.println(preparedURL);
  char Buf[200];
  preparedURL.toCharArray(Buf, 200);
  char *url = Buf;
  byte *data = (byte *)calloc(strlen(url) * 2, sizeof(byte));
  size_t len = 0;
  int res = convert_bits(data, &len, 5, (byte *)url, strlen(url), 8, 1);
  char *charLnurl = (char *)calloc(strlen(url) * 2, sizeof(byte));
  bech32_encode(charLnurl, "lnurl", data, len);
  to_upper(charLnurl);
  lnurl = charLnurl;
  Serial.println(lnurl);
}

/*
 * Fills output with nonce, xored payload, and HMAC.
 * XOR is secure for data smaller than the key size (it's basically one-time-pad). For larger data better to use AES.
 * Maximum length of the output in XOR mode is 1+1+nonce_len+1+32+8 = nonce_len+43 = 51 for 8-byte nonce.
 * Payload contains pin, currency byte and amount. Pin and amount are encoded as compact int (varint).
 * Currency byte is '$' for USD cents, 's' for satoshi, 'E' for euro cents.
 * Returns number of bytes written to the output, 0 if error occured.
 */
int xor_encrypt(uint8_t * output, size_t outlen, uint8_t * key, size_t keylen, uint8_t * nonce, size_t nonce_len, uint64_t pin, uint64_t amount_in_cents){
  // check we have space for all the data:
  // <variant_byte><len|nonce><len|payload:{pin}{amount}><hmac>
  if(outlen < 2 + nonce_len + 1 + lenVarInt(pin) + 1 + lenVarInt(amount_in_cents) + 8){
    return 0;
  }
  int cur = 0;
  output[cur] = 1; // variant: XOR encryption
  cur++;
  // nonce_len | nonce
  output[cur] = nonce_len;
  cur++;
  memcpy(output+cur, nonce, nonce_len);
  cur += nonce_len;
  // payload, unxored first - <pin><currency byte><amount>
  int payload_len = lenVarInt(pin) + 1 + lenVarInt(amount_in_cents);
  output[cur] = (uint8_t)payload_len;
  cur++;
  uint8_t * payload = output+cur; // pointer to the start of the payload
  cur += writeVarInt(pin, output+cur, outlen-cur); // pin code
  cur += writeVarInt(amount_in_cents, output+cur, outlen-cur); // amount
  cur++;
  // xor it with round key
  uint8_t hmacresult[32];
  SHA256 h;
  h.beginHMAC(key, keylen);
  h.write((uint8_t *)"Round secret:", 13);
  h.write(nonce, nonce_len);
  h.endHMAC(hmacresult);
  for(int i=0; i < payload_len; i++){
    payload[i] = payload[i] ^ hmacresult[i];
  }
  // add hmac to authenticate
  h.beginHMAC(key, keylen);
  h.write((uint8_t *)"Data:", 5);
  h.write(output, cur);
  h.endHMAC(hmacresult);
  memcpy(output+cur, hmacresult, 8);
  cur += 8;
  // return number of bytes written to the output
  return cur;
}
