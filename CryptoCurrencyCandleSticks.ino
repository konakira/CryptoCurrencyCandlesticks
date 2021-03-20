#if defined(ARDUINO_M5Stick_C_Plus) || defined(ARDUINO_M5Stick_C)
#include <M5StickCPlus.h>
#else // !ARDUINO_M5Stick_C_Plus
#ifdef ARDUINO_M5STACK_Core2
#include <M5Core2.h>
#else // !ARDUINO_M5STACK_Core2
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#endif // !ARDUINO_M5STACK_Core2
#endif // !ARDUINO_M5Stick_C_Plus
// #include <WiFi.h>
#include <WiFiClientSecure.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <SPI.h>
#include <SimpleTimer.h>
#include <TimeLib.h>

#include "Free_Fonts.h"
#include "auth.h"

const char* bitbank_root_ca= \
     "-----BEGIN CERTIFICATE-----\n" \
     "MIIEDzCCAvegAwIBAgIBADANBgkqhkiG9w0BAQUFADBoMQswCQYDVQQGEwJVUzEl\n" \
     "MCMGA1UEChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjEyMDAGA1UECxMp\n" \
     "U3RhcmZpZWxkIENsYXNzIDIgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMDQw\n" \
     "NjI5MTczOTE2WhcNMzQwNjI5MTczOTE2WjBoMQswCQYDVQQGEwJVUzElMCMGA1UE\n" \
     "ChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjEyMDAGA1UECxMpU3RhcmZp\n" \
     "ZWxkIENsYXNzIDIgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwggEgMA0GCSqGSIb3\n" \
     "DQEBAQUAA4IBDQAwggEIAoIBAQC3Msj+6XGmBIWtDBFk385N78gDGIc/oav7PKaf\n" \
     "8MOh2tTYbitTkPskpD6E8J7oX+zlJ0T1KKY/e97gKvDIr1MvnsoFAZMej2YcOadN\n" \
     "+lq2cwQlZut3f+dZxkqZJRRU6ybH838Z1TBwj6+wRir/resp7defqgSHo9T5iaU0\n" \
     "X9tDkYI22WY8sbi5gv2cOj4QyDvvBmVmepsZGD3/cVE8MC5fvj13c7JdBmzDI1aa\n" \
     "K4UmkhynArPkPw2vCHmCuDY96pzTNbO8acr1zJ3o/WSNF4Azbl5KXZnJHoe0nRrA\n" \
     "1W4TNSNe35tfPe/W93bC6j67eA0cQmdrBNj41tpvi/JEoAGrAgEDo4HFMIHCMB0G\n" \
     "A1UdDgQWBBS/X7fRzt0fhvRbVazc1xDCDqmI5zCBkgYDVR0jBIGKMIGHgBS/X7fR\n" \
     "zt0fhvRbVazc1xDCDqmI56FspGowaDELMAkGA1UEBhMCVVMxJTAjBgNVBAoTHFN0\n" \
     "YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4xMjAwBgNVBAsTKVN0YXJmaWVsZCBD\n" \
     "bGFzcyAyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5ggEAMAwGA1UdEwQFMAMBAf8w\n" \
     "DQYJKoZIhvcNAQEFBQADggEBAAWdP4id0ckaVaGsafPzWdqbAYcaT1epoXkJKtv3\n" \
     "L7IezMdeatiDh6GX70k1PncGQVhiv45YuApnP+yz3SFmH8lU+nLMPUxA2IGvd56D\n" \
     "eruix/U0F47ZEUD0/CwqTRV/p2JdLiXTAAsgGh1o+Re49L2L7ShZ3U0WixeDyLJl\n" \
     "xy16paq8U4Zt3VekyvggQQto8PT7dL5WXXp59fkdheMtlb71cZBDzI0fmgAKhynp\n" \
     "VSJYACPq4xJDKVtHCN2MQWplBqjlIapBtJUhlbl90TSrE9atvNziPTnNvT51cKEY\n" \
     "WQPJIrSPnNVeKtelttQKbfi3QBFGmh95DmK/D5fs4C8fF5Q=\n" \
     "-----END CERTIFICATE-----\n";

#define HORIZONTAL_RESOLUTION 240 // width of TTGO-T-display

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

#if defined(ARDUINO_M5Stick_C_Plus) || defined(ARDUINO_M5STACK_Core2)
#define LCD M5.Lcd
#else
#define LCD tft
#endif

WiFiClientSecure client;

SimpleTimer timer;

#define SERVER "public.bitbank.cc"

/*
  function to convert unsigned integer to comma separated numerical string
*/

void
itocsa(char *buf, unsigned bufsiz, unsigned n)
{
  char *p = buf;
  unsigned len, i;

  for (i = 1 ; i < n ; i *= 1000);

  if (1 < i) {
    i /= 1000;
    snprintf(p, bufsiz, "%d", n / i);
    n = n % i;
    len = strlen(p);
    p += len;
    bufsiz -= len;
    i /= 1000;
    while (0 < i) {
      snprintf(p, bufsiz, ",%03d", n / i);
      n = n % i;
      p += 4;
      bufsiz -= 4;
      i /= 1000;
    }
  }
  else {
    snprintf(p, bufsiz, "%d", n);
  }
}

#define CANDLESTICK_WIDTH "5min"
#define CANDLESTICK_WIDTH_MIN 5

#define BUTTON1 35 // GPIO35
#define BUTTON2 0 // GPIO0

#define STICK_WIDTH 3 // width of a candle stick
#define NUM_STICKS (HORIZONTAL_RESOLUTION / STICK_WIDTH)

class Currency {
public:
  char *name, *pair;
  unsigned priceline;
  struct {
    unsigned startPrice, endPrice, lowestPrice, highestPrice;
    float relative;
    unsigned long timeStamp;
  } candlesticks[NUM_STICKS];
  unsigned todayshigh = 0;
  unsigned todayslow = 0;
  unsigned prevPrice = 0, price = 0;
  unsigned lowest, highest;
  unsigned long prevTimeStamp = 0;
  float relative = 0.0, prevRelative = 0.0;
  float highestRelative, lowestRelative;
  int pricePixel;

  Currency (char *na, char *pa, unsigned pl) {
    name = na;
    pair = pa;
    priceline = pl;
  }

  unsigned obtainLastPrice(unsigned long *t);
  void obtainSticks(unsigned n, unsigned long t);
  void obtainSticks(unsigned n, unsigned long t, unsigned long lastTimeStamp);
  void calcRelative();
  void ShowChart(int yoff);
  void ShowCurrentPrice(int yoff);
  void SwitchCurrency();
  void ShowCurrencyName(char *buf, int yoff);
  void ShowUpdating(int yoff);
} currencies[2] = {{"ETH", "eth_jpy", 5000}, {"BTC", "btc_jpy", 100000}};

static unsigned cIndex = 0; // ETH by default.

unsigned numScreens = 2;
unsigned tftHeight = 0;
unsigned tftWidth = 0;
unsigned tftHalfHeight = 0;

#define TIMEZONE (9 * 60 * 60)

void
Currency::obtainSticks(unsigned n, unsigned long t, unsigned long lastTimeStamp)
{
  // Serial.println("\nobtainSticks called.");

  while (0 < n) {
    if (!client.connect(SERVER, 443)) {
      Serial.println("\nConnection failed!");
      return;
    }
    else {
      Serial.println("\nConnected to http server.");

      // Make another HTTP request:
      client.print("GET https://" SERVER "/");
      client.print(pair);
      client.print("/candlestick/" CANDLESTICK_WIDTH "/");
      {
	char yyyymmdd[9]; // 9 for "yyyymmdd"
	sprintf(yyyymmdd, "%04d%02d%02d", year(t), month(t), day(t));
	client.print(yyyymmdd);
	client.println(" HTTP/1.0");
      }
      client.println("Host: " SERVER);
      client.println("Connection: close");
      client.println();
    
// #define SHOW_HTTPHEADERS
    
      while (client.connected()) {
	String line = client.readStringUntil('\n');
#ifdef SHOW_HTTPHEADERS
	Serial.println(line); // echo response headers
#endif
	if (line == "\r") {
	  // Serial.println("headers received");
	  break;
	}
      }

      DynamicJsonDocument doc(50000);
      DeserializationError error = deserializeJson(doc, client);
      client.stop();

      if (error) {
	Serial.print(F("deserializeJson() failed: "));
	Serial.println(error.f_str());
      }
      else {
	unsigned lastIndex;
	int success = doc["success"]; // 1

	JsonArray ohlcv = doc["data"]["candlestick"][0]["ohlcv"];
	unsigned numSticks = ohlcv.size();

	// Serial.print("Success = ");
	// Serial.println(success);

	if (0 < lastTimeStamp) { // skip newer candlestick if any.
	  for (int i = numSticks - 1 ; 0 < i ; i--) {
	    // this may just remove one data at most
	    if ((unsigned long)(ohlcv[i][5].as<unsigned long long>() / 1000) <= lastTimeStamp) {
	      numSticks = i + 1;
	      break;
	    }
	  }
	}
	
	Serial.print("Number of sticks = ");
	Serial.print(numSticks);
	if (n <= numSticks) { // enough sticks obtained
	  Serial.println(" (enough)");
	  for (unsigned i = 0 ; i < n ; i++) {
	    // copy the last n data from JSON
	    unsigned ohlcvIndex = i + numSticks - n;
	    candlesticks[i].startPrice = ohlcv[ohlcvIndex][0].as<unsigned>();
	    candlesticks[i].highestPrice = ohlcv[ohlcvIndex][1].as<unsigned>();
	    candlesticks[i].lowestPrice = ohlcv[ohlcvIndex][2].as<unsigned>();
	    candlesticks[i].endPrice = ohlcv[ohlcvIndex][3].as<unsigned>();
	    candlesticks[i].timeStamp =
	      (unsigned long)(ohlcv[ohlcvIndex][5].as<unsigned long long>() / 1000);
	  }
	  n = 0;
	}
	else {
	  Serial.println(" (not enough)");
	  for (unsigned i = 0 ; i < numSticks ; i++) {
	    // copy the all n data from JSON
	    unsigned stickIndex = i + n - numSticks;
	    candlesticks[stickIndex].startPrice = ohlcv[i][0].as<unsigned>();
	    candlesticks[stickIndex].highestPrice = ohlcv[i][1].as<unsigned>();
	    candlesticks[stickIndex].lowestPrice = ohlcv[i][2].as<unsigned>();
	    candlesticks[stickIndex].endPrice = ohlcv[i][3].as<unsigned>();
	    candlesticks[stickIndex].timeStamp =
	      (unsigned long)(ohlcv[i][5].as<unsigned long long>() / 1000);
	  }
	  n -= numSticks; // to fill remaining slots
	  t -= 24 * 60 * 60; // for data one day before
	}
      }
    }
  }
  // obtaining chart's high and low
  lowest = candlesticks[0].lowestPrice; // chart's low
  highest = candlesticks[0].highestPrice; // chart's high
  for (unsigned i = 1 ; i < NUM_STICKS ; i++) {
    if (candlesticks[i].lowestPrice < lowest) {
      lowest = candlesticks[i].lowestPrice;
    }
    if (highest < candlesticks[i].highestPrice) {
      highest = candlesticks[i].highestPrice;
    }
  }
  if (todayshigh == 0) {
    unsigned today = day(candlesticks[NUM_STICKS - 1].timeStamp + TIMEZONE);
    
    for (unsigned i = 0 ; i < NUM_STICKS ; i++) {
      if (day(candlesticks[i].timeStamp + TIMEZONE) == today) {
	if (todayshigh == 0) {
	  todayshigh = candlesticks[i].highestPrice;
	  todayslow = candlesticks[i].lowestPrice;
	}
	else if (todayshigh < candlesticks[i].highestPrice) {
	  todayshigh = candlesticks[i].highestPrice;
	}
	else if (candlesticks[i].lowestPrice < todayslow) {
	  todayslow = candlesticks[i].lowestPrice;
	}
      }
    }
  }
  Serial.print("\ntoday = ");
  Serial.print(todayslow);
  Serial.print(" - ");
  Serial.println(todayshigh);
  Serial.print("chart = ");
  Serial.print(lowest);
  Serial.print(" - ");
  Serial.println(highest);
}

void
Currency::obtainSticks(unsigned n, unsigned long t)
{
  obtainSticks(n, t, 0);
}


// Output timestamp to serial terminal
void
SerialPrintTimestamp(unsigned t, unsigned tz)
{
  Serial.print("timestamp = ");
  Serial.print(t);
  t += tz;
  Serial.print(" = ");
  Serial.print(year(t));
  Serial.print("/");
  Serial.print(month(t));
  Serial.print("/");
  Serial.print(day(t));
  Serial.print(" ");
  Serial.print(hour(t));
  Serial.print(":");
  if (minute(t) < 10) {
    Serial.print("0");
  }
  Serial.print(minute(t));
  Serial.print(":");
  if (second(t) < 10) {
    Serial.print("0");
  }
  Serial.print(second(t));
  Serial.println(" JST");
}

unsigned
Currency::obtainLastPrice(unsigned long *t)
{
  if (!client.connect(SERVER, 443)) {
    Serial.println("Connection failed!");
  }
  else {
    Serial.println("Connected to http server.");
    // Make a HTTP request:
    client.print("GET https://" SERVER "/");
    client.print(pair);
    client.println("/ticker HTTP/1.0");
    client.println("Host: " SERVER);
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
#ifdef SHOW_HTTPHEADERS
      Serial.println(line); // echo response headers
#endif
      if (line == "\r") {
        // Serial.println("headers received");
        break;
      }
    }

    // Allocate the JSON document
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<256> doc;

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, client);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }
    else {
      // Extract values
      if (doc["success"]) {
	unsigned long timestamp;

	// Obtaining time stamp of ticker response and use it as the current time,
	// instead of obtaining current time by NTP.
	timestamp = (unsigned long)(doc["data"]["timestamp"].as<unsigned long long>() / 1000);
	if (day(timestamp + TIMEZONE) != day(prevTimeStamp + TIMEZONE)) {
	  todayshigh = 0;
	}
	prevTimeStamp = timestamp;
	SerialPrintTimestamp(timestamp, TIMEZONE);
	prevPrice = price;
	price = (unsigned)doc["data"]["last"].as<long>();
      }
    }
    client.stop();
  }
  if (t) {
    *t = prevTimeStamp;
  }
  return price;
}

#define ALERT_INTERVAL 500 // msec    
#define ALERT_DURATION (40 /* sec */ * (1000 / ALERT_INTERVAL)) // times ALERT_INTERVAL
static unsigned alertDuration = 0;
static unsigned long prevCandlestickTimestamp = 0;
static bool changeTriggered = false;

#define PRICEBUFSIZE 24

#define PRICE_MIN_X 5
#define PRICE_PAD_X 3
#define PRICE_PAD_Y 10
#define BORDER_WIDTH 2

#define PRICE_FONT FF44 // 20, 24, (36,) 44 are candidates for a price font
  
#define OTHER_CURRENCY_BASE_VALUE_FONT 4
#define TFT_DOWNRED 0xC000 /* 127,   0,   0 */
#define TFT_UPGREEN 0x0600 /*   0, 128,   0 */
// #define TFT_RED         0xF800      /* 255,   0,   0 */
// #define TFT_GREEN       0x07E0      /*   0, 255,   0 */

#define ONEMINUTE_THRESHOLD 1 // per cent
#define FIVEMINUTES_THRESHOLD 1 // per cent
#define PADX 5
#define PADY 5
#define MESGSIZE 64
#define ALERT_BLACK_DURATION 200 // msec

void
DrawStringWithShade(char *buf, int x, int y, unsigned font, int color, int shade)
{
  LCD.setTextColor(TFT_BLACK);
  LCD.drawString(buf, x - shade, y - shade, font);
  LCD.drawString(buf, x + shade, y + shade, font);
  LCD.setTextColor(color);
  LCD.drawString(buf, x, y, font);
}

void
ShowLastPrice(char *buf, int lastPricePixel, unsigned priceColor, int yoff)
{
  int textY = lastPricePixel - (LCD.fontHeight(GFXFF) / 2);
  if (textY < 0) {
    textY = 0;
  }
  else if (tftHeight - LCD.fontHeight(GFXFF) + PRICE_PAD_Y < textY) {
    textY = tftHeight - LCD.fontHeight(GFXFF) + PRICE_PAD_Y;
  }
  DrawStringWithShade(buf, 0, textY + yoff, GFXFF, priceColor, BORDER_WIDTH);
}

#define BASE_DIFF 4

void
ShowRelativePrice(char *buf, int lastPricePixel, unsigned priceColor, int yoff)
{
  int textY = lastPricePixel - (LCD.fontHeight(GFXFF) / 2);
  if (textY < 0) {
    textY = 0;
  }
  else if (tftHeight - LCD.fontHeight(GFXFF) + PRICE_PAD_Y < textY) {
    textY = tftHeight - LCD.fontHeight(GFXFF) + PRICE_PAD_Y;
  }
  if (lastPricePixel < tftHalfHeight) {
    textY += LCD.fontHeight(GFXFF);
  }
  else {
    textY -= LCD.fontHeight(OTHER_CURRENCY_BASE_VALUE_FONT);
  }
  DrawStringWithShade(buf, 0, textY + yoff, OTHER_CURRENCY_BASE_VALUE_FONT, priceColor, BORDER_WIDTH);
  DrawStringWithShade(currencies[cIndex == 0 ? 1 : 0].name,
		      PADX + LCD.textWidth(buf, OTHER_CURRENCY_BASE_VALUE_FONT),
		      textY + yoff + LCD.fontHeight(OTHER_CURRENCY_BASE_VALUE_FONT) - LCD.fontHeight(2) - BASE_DIFF, 2, priceColor, BORDER_WIDTH);
}

void
Currency::ShowCurrencyName(char *buf, int yoff)
{
  int textY;
  if (pricePixel < tftHalfHeight) {
    textY = tftHeight - LCD.fontHeight(2) * 2;
  }
  else {
    textY = LCD.fontHeight(2);
  }
  LCD.setTextColor(TFT_WHITE);
  LCD.drawString(buf, tftWidth - LCD.textWidth(buf, 2) - 1, textY + yoff, 2);
}

static char *updating = "Updating...";

void
Currency::ShowUpdating(int yoff)
{
  int textY;
  if (pricePixel < tftHalfHeight) {
    textY = tftHeight - LCD.fontHeight(2) * 2;
  }
  else {
    textY = LCD.fontHeight(2);
  }
  LCD.setTextColor(TFT_WHITE, TFT_BLUE);
  LCD.drawString(updating, tftWidth - LCD.textWidth(updating, 2) - 1, textY + yoff, 2);
}

class alert {
public:
  void setBackColor(unsigned color) {
    alertbgcolor = color;
  }
  char mesgbuf[MESGSIZE];
  int alertId;
  void setMesg1(char *s) {
    alertmesg1 = s;
  }
  void setMesg2(char *s) {
    alertmesg2 = s;
  }
  void setLastPrice(unsigned p) {
    lastPrice = p;
  }
  void beginAlert() {
    LCD.fillScreen(alertbgcolor);
    LCD.setTextColor(TFT_WHITE);
    itocsa(buf, PRICEBUFSIZE, lastPrice);
    textColor = TFT_WHITE;
    showAlert();
  }
  void flashAlert() {
    textColor = (textColor == TFT_WHITE) ? TFT_BLACK : TFT_WHITE;
    LCD.setTextColor(textColor);
    showAlert();
  }
private:
  void showAlert() {
    LCD.drawString(alertmesg1, PADX, PADY, 4);
    LCD.drawString(alertmesg2, PADX, LCD.fontHeight(4) + PADY, 4);
    LCD.drawString(buf,
		   tftWidth / 2 - LCD.textWidth(buf, GFXFF) / 2,
		   tftHeight - LCD.fontHeight(GFXFF), GFXFF);
  }
  char buf[PRICEBUFSIZE];
  char *alertmesg1;
  char *alertmesg2;
  unsigned textColor = TFT_WHITE;
  unsigned alertbgcolor = TFT_DOWNRED; // alert color by default
  unsigned lastPrice;
} Alert;

int
floatmap(float val, float l, float h, int a, int b)
{
  double res = (val - l) / (h - l);
  int height = b - a;

  return (int)(res * height) + a;
}

void
Currency::ShowChart(int yoff)
{
  char buf[PRICEBUFSIZE], buf2[PRICEBUFSIZE];
  unsigned stickColor = TFT_DOWNRED, priceColor = TFT_GREEN;

  if (0 < alertDuration) {
    return;
  }

  // show the chart
  
  LCD.fillScreen(TFT_BLACK);

#define TFT_DARKBLUE        0x000F      /*   0,   0, 127 */

  // draw horizontal price lines
  if (price < prevPrice) {
    priceColor = TFT_RED;
  }
  for (unsigned i = lowest / priceline + 1 ; i * priceline < highest ; i++) {
    int y = map(i * priceline, lowest, highest, tftHeight, 0);
    LCD.drawFastHLine(0, y + yoff, tftWidth, TFT_DARKBLUE);
  }

  // get the position to draw last price
  pricePixel = map(price, lowest, highest, tftHeight, 0);

  // get initial position for relative price
  unsigned prevRel = floatmap(candlesticks[0].relative, lowestRelative, highestRelative, tftHeight, 0);
  unsigned prevHour = hour(candlesticks[0].timeStamp + TIMEZONE);
    
  itocsa(buf, PRICEBUFSIZE, highest);

  // draw candlesticks
  for (unsigned i = 0 ; i < NUM_STICKS ; i++) {
    // draw vertical hour line
    unsigned curHour = hour(candlesticks[i].timeStamp + TIMEZONE);
    if (curHour != prevHour) {
      prevHour = curHour;
      LCD.drawFastVLine(i * 3 + 1, yoff, tftHeight, TFT_DARKBLUE);
      if (curHour % 3 == 0) {
	char bufHour[4];
	snprintf(bufHour, sizeof(bufHour) - 1, "%d", curHour);
	int offset = LCD.textWidth(bufHour, 2) / 2;
	if (i * 3 < tftWidth - LCD.textWidth(buf, 2) - offset - PADX) {
	  // if we have enough space around vertical line, draw the time
	  unsigned textY = 0;

	  if (pricePixel < tftHalfHeight) {
	    textY = tftHeight - LCD.fontHeight(2);
	  }
	  LCD.setTextColor(TFT_CYAN);
	  LCD.drawNumber(curHour, i * 3 - offset, textY + yoff, 2);
	}
      }
    }

    if (0 < i) { // draw graph for relative prices
      unsigned curRel = floatmap(candlesticks[i].relative, lowestRelative, highestRelative,
				 tftHeight, 0);
      LCD.drawLine(i * 3 - 2, prevRel + yoff, i * 3 + 1, curRel + yoff, TFT_ORANGE);
      prevRel = curRel;
    }

    // draw candlesticks
    int lowestPixel, highestPixel, lowPixel, pixelHeight;
    
    lowestPixel = map(candlesticks[i].lowestPrice, lowest, highest, tftHeight, 0);
    highestPixel = map(candlesticks[i].highestPrice, lowest, highest, tftHeight, 0);

    if (candlesticks[i].startPrice < candlesticks[i].endPrice) {
      lowPixel = map(candlesticks[i].endPrice, lowest, highest, tftHeight, 0);
      pixelHeight = map(candlesticks[i].startPrice, lowest, highest, tftHeight, 0)
	- lowPixel;
      stickColor = TFT_UPGREEN;
    }
    else {
      lowPixel = map(candlesticks[i].startPrice, lowest, highest, tftHeight, 0);
      pixelHeight = map(candlesticks[i].endPrice, lowest, highest, tftHeight, 0)
	- lowPixel;
      stickColor = TFT_DOWNRED;
    }

    LCD.drawFastVLine(i * 3 + 1, highestPixel + yoff, lowestPixel - highestPixel, TFT_LIGHTGREY);
    LCD.fillRect(i * 3, highestPixel + yoff, 3, pixelHeight, stickColor);
  }

  // draw price horizontal line
  LCD.drawFastHLine(0, pricePixel + yoff, tftWidth, priceColor);

  // draw highest and lowest price in the chart
  itocsa(buf, PRICEBUFSIZE, highest);
  DrawStringWithShade(buf, tftWidth - LCD.textWidth(buf, 2) - 1, yoff, 2, TFT_WHITE, 1);

  itocsa(buf, PRICEBUFSIZE, lowest);

  DrawStringWithShade(buf, tftWidth - LCD.textWidth(buf, 2) - 1, tftHeight + yoff, 2, TFT_WHITE, 1);

  // show currency name
  ShowCurrencyName(name, yoff);
  
  // draw last price
  itocsa(buf, PRICEBUFSIZE, price);

  // show the current cryptocurrency price on TTGO-T-display

  ShowLastPrice(buf, pricePixel, priceColor, yoff);
  snprintf(buf2, PRICEBUFSIZE, "%.5f", relative);
  ShowRelativePrice(buf2, pricePixel,
		    relative < prevRelative ? TFT_RED: TFT_GREEN, yoff);
  prevRelative = relative;
}

void
Currency::calcRelative()
{
  unsigned another = (cIndex == 0 ? 1 : 0);

  highestRelative = lowestRelative = 
    candlesticks[0].relative = (float)candlesticks[0].endPrice / (float)currencies[another].candlesticks[0].endPrice;
  for (unsigned i = 1 ; i < NUM_STICKS ; i++) {
    float re = 0.0;
    if (0 < currencies[another].candlesticks[i].endPrice) {
      re = (float)candlesticks[i].endPrice / (float)currencies[another].candlesticks[i].endPrice;
    }

    candlesticks[i].relative = re;
    if (re < lowestRelative) {
      lowestRelative = re;
    }
    if (highestRelative < re) {
      highestRelative = re;
    }
  }
}

void
Currency::ShowCurrentPrice(int yoff)
{
  unsigned long t; // for current time
  unsigned long prevTime;
  char buf[PRICEBUFSIZE], buf2[PRICEBUFSIZE];
  unsigned stickColor = TFT_DOWNRED, priceColor = TFT_GREEN;

  if (0 < alertDuration) {
    return;
  }

  client.setCACert(bitbank_root_ca);

  if (WiFi.status() != WL_CONNECTED) {
    //WiFi.disconnect();

    // Grey out the price display
    itocsa(buf, PRICEBUFSIZE, prevPrice);
    ShowLastPrice(buf, pricePixel, TFT_DARKGREY, yoff); // make the price grey
    snprintf(buf2, PRICEBUFSIZE, "%.5f", relative);
    ShowRelativePrice(buf2, pricePixel, TFT_DARKGREY, yoff);

#define CONNECTION_LOST "Reconnecting ..."
    LCD.setTextColor(TFT_WHITE, TFT_BLUE);
    LCD.drawString(CONNECTION_LOST,
		   tftWidth / 2 - LCD.textWidth(CONNECTION_LOST, 4) / 2,
		   tftHalfHeight - LCD.fontHeight(4) / 2, 4);
    
    Serial.print("WiFi connection was lost.\nAttempting to reconnect to WiFi ");
    // WiFi.begin(WIFIAP, WIFIPW);
    WiFi.reconnect();

#define WIFI_ATTEMPT_LIMIT 45
    unsigned i;
    
    // attempt to connect to Wifi network:
    for(i = 0 ; i < WIFI_ATTEMPT_LIMIT && WiFi.status() != WL_CONNECTED ; i++) {
      Serial.print(".");
      // wait 1 second for re-trying
      delay(1000);
    }
    if (i < WIFI_ATTEMPT_LIMIT) {
      Serial.println(" Connected");
    }
    else {
      Serial.println(" Failed to recoonect");
      return;
    }
  }
  
  Serial.println("\n==== Starting connection to server...");

  ShowUpdating(yoff);

  prevTime = prevTimeStamp;
  unsigned lastPriceOfOtherCurrency = currencies[cIndex == 0 ? 1 : 0].obtainLastPrice(&t);
  obtainLastPrice(&t);
  relative = (float)price / (float)lastPriceOfOtherCurrency;
  itocsa(buf, PRICEBUFSIZE, price);
  Serial.print("last price = ");
  Serial.println(buf);
  // obtaining today's low and today's high
  if (0 < price
      && (0 == prevTime
	  || (minute(prevTimeStamp) % CANDLESTICK_WIDTH_MIN) < (minute(prevTime) % CANDLESTICK_WIDTH_MIN))) {
    // I forgot what '0 < price' means
    // Meaning of '(minute(prevTimeStamp) % CANDLESTICK_WIDTH_MIN) < (minute(prevTime) % CANDLESTICK_WIDTH_MIN)'
    // is that time exceeds specified CANDLESTICK_WIDTH_MIN, so that, it is necessary to obtain candlesticks.
    obtainSticks(NUM_STICKS, t);
    // get data for another currency
    unsigned another = (cIndex == 0 ? 1 : 0);
    currencies[another].obtainSticks(NUM_STICKS, t, candlesticks[NUM_STICKS - 1].timeStamp);
    calcRelative();
  }

  SerialPrintTimestamp(candlesticks[NUM_STICKS - 1].timeStamp, TIMEZONE);

  Serial.print("Vertical price line in pixel = ");
  Serial.println(map(candlesticks[NUM_STICKS - 1].endPrice, lowest, highest, tftHeight, 0));

  // check events...
  // last event has the highest priority

  // today's high or low
  if (price < todayslow) {
    todayslow = price;

    Alert.setMesg1("Updated");
    Alert.setMesg2("today's low");
    Alert.setBackColor(TFT_DOWNRED);
    alertDuration = ALERT_DURATION;
  }
  else if (todayshigh < price) {
    todayshigh = price;

    Alert.setMesg1("Updated");
    Alert.setMesg2("today's high");
    Alert.setBackColor(TFT_UPGREEN);
    alertDuration = ALERT_DURATION;
  }

  // not event but set today's high and low properly
  if (todayshigh < candlesticks[NUM_STICKS - 1].highestPrice) {
    todayshigh = candlesticks[NUM_STICKS - 1].highestPrice;
  }
  if (candlesticks[NUM_STICKS - 1].lowestPrice < todayslow) {
    todayslow = candlesticks[NUM_STICKS - 1].lowestPrice;
  }

  // five minutes significant price change
  if (prevCandlestickTimestamp != candlesticks[NUM_STICKS - 1].timeStamp &&
	   FIVEMINUTES_THRESHOLD <=
	   abs((long)candlesticks[NUM_STICKS -1].startPrice - (long)candlesticks[NUM_STICKS -1].endPrice)
	   * 100 / (long)candlesticks[NUM_STICKS - 1].startPrice) {
    prevCandlestickTimestamp = candlesticks[NUM_STICKS - 1].timeStamp;
    if (candlesticks[NUM_STICKS - 1].startPrice < candlesticks[NUM_STICKS - 1].endPrice) {
      Alert.setBackColor(TFT_UPGREEN);
      snprintf(Alert.mesgbuf, MESGSIZE, "%.1f%% up within",
	       (float)(candlesticks[NUM_STICKS -1 ].endPrice - candlesticks[NUM_STICKS - 1].startPrice) * 100.0
	       / (float)candlesticks[NUM_STICKS - 1].startPrice);
    }
    else {
      Alert.setBackColor(TFT_DOWNRED);
      snprintf(Alert.mesgbuf, MESGSIZE, "%.1f%% down within",
	       (float)(candlesticks[NUM_STICKS -1 ].startPrice - candlesticks[NUM_STICKS - 1].endPrice) * 100.0
	       / (float)candlesticks[NUM_STICKS - 1].startPrice); 
    }
    Alert.setMesg1(Alert.mesgbuf);
    Alert.setMesg2("5 minutes.");
    alertDuration = ALERT_DURATION;
  }

  // in a minute significant price change
  if (0 < prevPrice && ONEMINUTE_THRESHOLD <= abs((long)price - (long)prevPrice) * 100 / (long)prevPrice) {
    if (prevPrice < price) {
      snprintf(Alert.mesgbuf, MESGSIZE, "%.1f%% up within",
	       (float)(price - prevPrice) * 100.0 / (float)prevPrice);
      Alert.setBackColor(TFT_UPGREEN);
    }
    else {
      snprintf(Alert.mesgbuf, MESGSIZE, "%.1f%% down within",
	       (float)(prevPrice - price) * 100.0 / (float)prevPrice);
      Alert.setBackColor(TFT_DOWNRED);
    }
    Alert.setMesg1(Alert.mesgbuf);
    Alert.setMesg2("a minute.");
    alertDuration = ALERT_DURATION;
  }

  if (0 < alertDuration) {
    Alert.setLastPrice(price);
    Alert.beginAlert();
    Alert.alertId = timer.setTimer(ALERT_INTERVAL, alertProc, ALERT_DURATION * (1000 / ALERT_INTERVAL) + 1);
  }
  else {
    // show the chart
    ShowChart(yoff);
  }
}

void
alertProc()
{
  if (0 < alertDuration) {
    Alert.flashAlert();
    alertDuration--;
    if (alertDuration == 0) {
      timer.deleteTimer(Alert.alertId);
      currencies[cIndex].ShowChart(0);
    }
  }
}

void Currency::SwitchCurrency()
{
  char buf[PRICEBUFSIZE], buf2[PRICEBUFSIZE];
  
  // clear global variables..
  prevCandlestickTimestamp = 0;
  alertDuration = 0;
    
  Serial.println("\nChange triggered.");

  // Grey out the price display
  itocsa(buf, PRICEBUFSIZE, price);
  ShowLastPrice(buf, pricePixel, TFT_DARKGREY, 0/* yoff */); // make the price grey
  snprintf(buf2, PRICEBUFSIZE, "%.5f", relative);
  ShowRelativePrice(buf2, pricePixel, TFT_DARKGREY, 0/* yoff */);

#define SWITCHING "Switching ..."
  LCD.setTextColor(TFT_WHITE, TFT_BLUE);
  LCD.drawString(SWITCHING,
		 tftWidth / 2 - LCD.textWidth(SWITCHING, 4) / 2,
		 tftHalfHeight - LCD.fontHeight(4) / 2, 4);
  
  cIndex = (cIndex == 0) ? 1 : 0;

  currencies[cIndex].relative = 1 / relative;
  currencies[cIndex].calcRelative();
  currencies[cIndex].ShowChart(0);
}

void SecProc()
{
  if (changeTriggered) {
    changeTriggered = false;
    currencies[cIndex].SwitchCurrency();
  }
}

void
_ShowCurrentPrice()
{
  currencies[cIndex].ShowCurrentPrice(tftHeight);
}

#if !defined(ARDUINO_M5Stick_C) && !defined(ARDUINO_M5Stick_C_Plus) && !defined(ARDUINO_M5STACK_Core2)
void buttonEventProc()
{
  changeTriggered = true;
}
#endif

void setup()
{
#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus) || defined(ARDUINO_M5STACK_Core2)
  // initialize the M5StickC object
  M5.begin();
  delay(500);
#else
  // initialize TFT screen
  tft.init(); // equivalent to tft.begin();
  Serial.begin(115200);
#endif

  Serial.println("");
  Serial.println("CryptoCurrency candlestick chart display terminal started.");

#ifdef ARDUINO_M5STACK_Core2
  M5.Lcd.setRotation(0); // set it to 1 or 3 for landscape resolution
  tftHeight = M5.Lcd.height() / numScreens;
  tftWidth = M5.Lcd.width();
  M5.Lcd.fillScreen(BLACK);
#else
  tft.setRotation(1); // set it to 1 or 3 for landscape resolution
  tftHeight = tft.height() / numScreens;
  tftWidth = tft.width();
#endif


  Serial.print("tftHeight = ");
  Serial.println(tftHeight);
  
  tftHalfHeight = tftHeight / 2;

  LCD.fillScreen(TFT_BLUE);
  LCD.setTextColor(TFT_WHITE);
  LCD.setTextPadding(PADX); // seems no effect by this line.
  LCD.drawString("Connecting ...",
		 LCD.padX, LCD.height() / 2 - LCD.fontHeight(4) / 2, 4);
  LCD.setTextSize(1);
  LCD.setFreeFont(PRICE_FONT); // Select a font for last price display

  Serial.print("Attempting to connect to WiFi ");
  WiFi.begin(WIFIAP, WIFIPW);
  //  WiFi.disconnect();
  //WiFi.stop();

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }
  Serial.println(" Connected");

  Serial.println("");

#if !defined(ARDUINO_M5Stick_C_Plus) && !defined(ARDUINO_M5STACK_Core2)
  attachInterrupt(digitalPinToInterrupt(BUTTON1), buttonEventProc, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON2), buttonEventProc, FALLING);
#endif
  
  timer.setInterval(1000, SecProc);
  timer.setInterval(60000, _ShowCurrentPrice);
  _ShowCurrentPrice();
}

void loop()
{
#if defined(ARDUINO_M5Stick_C_Plus) || defined(ARDUINO_M5STACK_Core2)
  M5.update();
  if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) {
    changeTriggered = true;
  }
#endif
  timer.run();
}
