#ifdef ARDUINO_M5Stick_C
#define ARDUINO_M5Stick_C_Plus // Just in case this is not defined for building for StickC Plus
#endif

#ifdef ARDUINO_M5Stick_C_Plus
#include <M5StickCPlus.h>
#else // !ARDUINO_M5Stick_C_Plus
#ifdef ARDUINO_M5Stick_C
#include <M5StickC.h>
#else // !ARDUINO_M5Stick_C
#ifdef ARDUINO_M5STACK_Core2
#include <M5Core2.h>
#else // !ARDUINO_M5STACK_Core2
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#endif // !ARDUINO_M5STACK_Core2
#endif // !ARDUINO_M5Stick_C
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

#define MAX_HORIZONTAL_RESOLUTION 321

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus) || defined(ARDUINO_M5STACK_Core2)
#define LCD M5.Lcd
#else
#undef LCD
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
#define NUM_STICKS (MAX_HORIZONTAL_RESOLUTION / STICK_WIDTH)

class Currency {
public:
  const char* name;
  const char* pair;
  int priceline;
  unsigned another = 0;
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

  Currency (const char *na, const char *pa, unsigned pl) {
    name = na;
    pair = pa;
    priceline = pl;
  }

  unsigned obtainLastPrice(unsigned long *t);
  void obtainSticks(unsigned n, unsigned long t);
  void obtainSticks(unsigned n, unsigned long t, unsigned long lastTimeStamp);
  void calcRelative();
  void ShowChart(int yoff);
  void ShowCurrentPrice(bool forceReloadSticks);
  void GreyoutPrice();
  void SwitchCurrency();
  void ShowCurrencyName(const char *buf, int yoff);
  void ShowUpdating(int yoff);
  void setAlert(class alert a);
} currencies[2] = {{"ETH", "eth_jpy", 5000}, {"BTC", "btc_jpy", 100000}};

static unsigned cIndex = 0; // ETH by default.

unsigned numScreens = 1;
unsigned tftHeight = 0;
unsigned tftWidth = 0;
unsigned tftHalfHeight = 0;
unsigned dedicatedPriceAreaHeight = 0;
unsigned PriceFontHeight = 0;
unsigned numSticks = 1;

#define TIMEZONE (9 * 60 * 60)
#define MINIMUM_SPLITTABLE_HEIGHT 239 // for splitting screen into dual one
#define MINIMUM_SEPARATABLE_HEIGHT 150 // for price and chart seperation
#define MINIMUM_WIDTH 199 // allowed minimum width used in rotation

void
Currency::obtainSticks(unsigned n, unsigned long t, unsigned long lastTimeStamp)
{
  // Serial.println("\nobtainSticks called.");

  while (0 < n) {
    if (client.connect(SERVER, 443)) {
      Serial.println("\nConnected to http server for sticks.");

// #define SHOW_HTTPHEADERS

      // HTTP request:
      client.print("GET https://" SERVER "/");
      client.print(pair);
      client.print("/candlestick/" CANDLESTICK_WIDTH "/");
#ifdef SHOW_HTTPHEADERS
      Serial.print("GET https://" SERVER "/");
      Serial.print(pair);
      Serial.print("/candlestick/" CANDLESTICK_WIDTH "/");
#endif
      {
	char yyyymmdd[9]; // 9 for "yyyymmdd"
	sprintf(yyyymmdd, "%04d%02d%02d", year(t), month(t), day(t));
	client.print(yyyymmdd);
	client.println(" HTTP/1.0");
#ifdef SHOW_HTTPHEADERS
	Serial.print(yyyymmdd);
	Serial.println(" HTTP/1.0");
#endif
      }
      client.println("Host: " SERVER);
      client.println("Connection: close");
      client.println();
#ifdef SHOW_HTTPHEADERS
      Serial.println("Host: " SERVER);
      Serial.println("Connection: close");
      Serial.println();
#endif

#define ERRTHRESHOLD 30
      unsigned errcounter = 0;
      while (client.connected() && errcounter < ERRTHRESHOLD) {
	String line = client.readStringUntil('\n'); // timeout in 1000msec
	if (line) {
#ifdef SHOW_HTTPHEADERS
	  Serial.println(line); // echo response headers
#endif
	  if (line == "\r") {
	    // Serial.println("headers received");
	    break;
	  }
	}
	else {
	  errcounter++;
	}
      }

      if (errcounter < ERRTHRESHOLD) {
	DynamicJsonDocument doc(50000);
	DeserializationError error = deserializeJson(doc, client);
	client.stop();

	if (!error) {
	  int success = doc["success"]; // 1

	  if (success) {
	    JsonArray ohlcv = doc["data"]["candlestick"][0]["ohlcv"];
	    unsigned nSticks = ohlcv.size();

	    // Serial.print("Success = ");
	    // Serial.println(success);

	    if (0 < lastTimeStamp) { // skip newer candlestick if any.
	      for (int i = nSticks - 1 ; 0 < i ; i--) {
		// this may just remove one data at most
		if ((unsigned long)(ohlcv[i][5].as<unsigned long long>() / 1000) <= lastTimeStamp) {
		  nSticks = i + 1;
		  break;
		}
	      }
	    }
      
	    Serial.print("Number of sticks = ");
	    Serial.print(nSticks);
	    if (n <= nSticks) { // enough sticks obtained
	      Serial.println(" (enough)");
	      for (unsigned i = 0 ; i < n ; i++) {
		// copy the last n data from JSON
		unsigned ohlcvIndex = i + nSticks - n;
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
	      for (unsigned i = 0 ; i < nSticks ; i++) {
		// copy the all n data from JSON
		unsigned stickIndex = i + n - nSticks;
		candlesticks[stickIndex].startPrice = ohlcv[i][0].as<unsigned>();
		candlesticks[stickIndex].highestPrice = ohlcv[i][1].as<unsigned>();
		candlesticks[stickIndex].lowestPrice = ohlcv[i][2].as<unsigned>();
		candlesticks[stickIndex].endPrice = ohlcv[i][3].as<unsigned>();
		candlesticks[stickIndex].timeStamp =
		  (unsigned long)(ohlcv[i][5].as<unsigned long long>() / 1000);
	      }
	      n -= nSticks; // to fill remaining slots
	      t -= 24 * 60 * 60; // for data one day before
	    }
	  }
	  else { // not success
	    Serial.println(F("deserializeJson() succeded w/ false 'success' flag."));
	    return;
	  }
	}
	else {
	  Serial.print(F("deserializeJson() failed: "));
	  Serial.println(error.f_str());
	  return;
	}
      }
      else {
	Serial.println(F("http read timedout."));
	return;
      }
    }
    else {
      Serial.println("\nConnection failed!");
      return;
    }
  }
  // obtaining chart's high and low
  lowest = candlesticks[0].lowestPrice; // chart's low
  highest = candlesticks[0].highestPrice; // chart's high
  for (unsigned i = 1 ; i < numSticks ; i++) {
    if (candlesticks[i].lowestPrice < lowest) {
      lowest = candlesticks[i].lowestPrice;
    }
    if (highest < candlesticks[i].highestPrice) {
      highest = candlesticks[i].highestPrice;
    }
  }
  if (todayshigh == 0) { // if 'todayshigh' is not set
    // set 'todayshigh' based on candlestick information
    // actual 'todayshigh' should be set using last price, out of this routine
    unsigned today = day(candlesticks[numSticks - 1].timeStamp + TIMEZONE);
    
    for (unsigned i = 0 ; i < numSticks ; i++) {
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
    Serial.println("Connected to http server for price.");
    // Make a HTTP request:
    client.print("GET https://" SERVER "/");
    client.print(pair);
    client.println("/ticker HTTP/1.0");
    client.println("Host: " SERVER);
    client.println("Connection: close");
    client.println();

    unsigned errcounter = 0;
    while (client.connected() && errcounter < ERRTHRESHOLD) {
      String line = client.readStringUntil('\n'); // timeout in 1000msec
      if (line) {
#ifdef SHOW_HTTPHEADERS
	Serial.println(line); // echo response headers
#endif
	if (line == "\r") {
	  // Serial.println("headers received");
	  break;
	}
      }
      else {
	errcounter++;
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
#define ALERT_DURATION (30 /* sec */ * (1000 / ALERT_INTERVAL)) // times ALERT_INTERVAL
static unsigned alertDuration = 0;
static unsigned long prevCandlestickTimestamp = 0;
static bool changeTriggered = false;
static bool rotationTriggered = false;
static bool currencyRotationTriggered = false;

#define PRICEBUFSIZE 24

#define PRICE_MIN_X 5
#define PRICE_PAD_X 3
#define PRICE_PAD_Y 10
#define BORDER_WIDTH 2

#if defined(ARDUINO_M5Stick_C) && !defined(ARDUINO_M5Stick_C_Plus)
#define PRICEFONT 4
#define OTHER_CURRENCY_BASE_VALUE_FONT 2
#define PRICE_FONT_HEIGHT_ADJUSTMENT 2
#define CONNECTINGFONT 4
#define BASE_DIFF 0 // base difference between relative price font and its unit font
#else
#define PRICE_FONT FF44 // 20, 24, (36,) 44 are candidates for a price font
#define PRICEFONT GFXFF
#define OTHER_CURRENCY_BASE_VALUE_FONT 4
#define PRICE_FONT_HEIGHT_ADJUSTMENT 10
#define CONNECTINGFONT 4
#define BASE_DIFF 4 // base difference between relative price font and its unit font
#endif
  
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
DrawStringWithShade(const char *buf, int x, int y, unsigned font, int color, int shade)
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
  int textY;
  if (dedicatedPriceAreaHeight) {
    textY = 0;
  }
  else {
    textY = lastPricePixel - (PriceFontHeight / 2);
    if (textY < 0) {
      textY = 0;
    }
    else if (tftHeight - PriceFontHeight + PRICE_PAD_Y < textY) {
      textY = tftHeight - PriceFontHeight + PRICE_PAD_Y;
    }
    textY += yoff;
  }
  DrawStringWithShade(buf, 0, textY, PRICEFONT, priceColor, BORDER_WIDTH);
}

void
ShowRelativePrice(char *buf, const char *name, int lastPricePixel, unsigned priceColor, int yoff)
{
  int textY;

  if (dedicatedPriceAreaHeight) {
    textY = PriceFontHeight;
  }
  else {
    textY = lastPricePixel - (PriceFontHeight / 2);
    if (textY < 0) {
      textY = 0;
    }
    else if (tftHeight - PriceFontHeight + PRICE_PAD_Y < textY) {
      textY = tftHeight - PriceFontHeight + PRICE_PAD_Y;
    }
    if (lastPricePixel < tftHalfHeight) {
      textY += PriceFontHeight;
    }
    else {
      textY -= LCD.fontHeight(OTHER_CURRENCY_BASE_VALUE_FONT);
    }
  }
  DrawStringWithShade(buf, PADX, textY + yoff, OTHER_CURRENCY_BASE_VALUE_FONT, priceColor, BORDER_WIDTH);
  DrawStringWithShade(name,
		      PADX + LCD.textWidth(buf, OTHER_CURRENCY_BASE_VALUE_FONT),
		      textY + yoff + LCD.fontHeight(OTHER_CURRENCY_BASE_VALUE_FONT) - LCD.fontHeight(2) - BASE_DIFF, 2, priceColor, BORDER_WIDTH);
}

void
Currency::ShowCurrencyName(const char *buf, int yoff)
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

static const char *updating = "Updating...";

void
Currency::ShowUpdating(int yoff)
{
  int textY;
  if (pricePixel < tftHalfHeight) {
    textY = tftHeight - LCD.fontHeight(2) * 2 + yoff;
  }
  else {
    textY = LCD.fontHeight(2);
  }
  LCD.setTextColor(TFT_WHITE, TFT_BLUE);
  LCD.drawString(updating, tftWidth - LCD.textWidth(updating, 2) - 1,
		 textY + dedicatedPriceAreaHeight, 2);
}

class alert {
public:
  void setBackColor(unsigned color) {
    alertbgcolor = color;
  }
  char mesgbuf[MESGSIZE];
  int alertId;
  void setMesg1(const char *s) {
    alertmesg1 = s;
  }
  void setMesg2(const char *s) {
    alertmesg2 = s;
  }
  void setLastPrice(unsigned p) {
    lastPrice = p;
  }
  void beginAlert() {
    //LCD.fillScreen(alertbgcolor);
    LCD.fillRect(0, 1 < numScreens ? tftHeight : 0 + dedicatedPriceAreaHeight,
		 tftWidth, tftHeight, alertbgcolor);
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
    unsigned yoff = 1 < numScreens ? tftHeight : 0 + dedicatedPriceAreaHeight;

    LCD.drawString(alertmesg1, PADX, PADY + yoff, 4);
    LCD.drawString(alertmesg2, PADX, LCD.fontHeight(4) + PADY + yoff, 4);
    LCD.drawString(buf,
		   tftWidth / 2 - LCD.textWidth(buf, PRICEFONT) / 2,
		   tftHeight - PriceFontHeight + yoff, PRICEFONT);
  }
  char buf[PRICEBUFSIZE];
  const char *alertmesg1;
  const char *alertmesg2;
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

  // show the chart

#define TFT_DARKBLUE        0x000F      /*   0,   0, 127 */

  yoff += dedicatedPriceAreaHeight;

  if (highest < price) {
    highest = price;
  }
  else if (price < lowest) {
    lowest = price;
  }
  
  // draw horizontal price lines
  if (price < prevPrice) {
    priceColor = TFT_RED;
  }
  for (int i = lowest / priceline + 1 ; i * priceline < highest ; i++) {
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
  for (unsigned i = 0 ; i < numSticks ; i++) {
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
				 tftHeight- 1, 2);
      // note that the lowest pixel is 2, instead of 0 to prevent collision in
      // dual screen mode
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

  DrawStringWithShade(buf, tftWidth - LCD.textWidth(buf, 2) - 1,
		      tftHeight + yoff - LCD.fontHeight(2), 2, TFT_WHITE, 1);

  // show currency name
  ShowCurrencyName(name, yoff);
  
  // draw last price
  itocsa(buf, PRICEBUFSIZE, price);

  // show the current cryptocurrency price on TTGO-T-display
  yoff -= dedicatedPriceAreaHeight;
  ShowLastPrice(buf, pricePixel, priceColor, yoff);
  snprintf(buf2, PRICEBUFSIZE, "%.5f", relative);
  ShowRelativePrice(buf2, currencies[another].name, pricePixel,
		    relative < prevRelative ? TFT_RED: TFT_GREEN, yoff);
}

void
Currency::calcRelative()
{
  highestRelative = lowestRelative = candlesticks[0].relative =
    (float)candlesticks[0].endPrice / (float)currencies[another].candlesticks[0].endPrice;
  // Do the same for another currency
  currencies[another].highestRelative = currencies[another].lowestRelative =
    currencies[another].candlesticks[0].relative =
    (float)currencies[another].candlesticks[0].endPrice / (float)candlesticks[0].endPrice;
  for (unsigned i = 1 ; i < numSticks ; i++) {
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
    // Do the same for another currency
    if (0 < candlesticks[i].endPrice) {
      re = (float)currencies[another].candlesticks[i].endPrice / (float)candlesticks[i].endPrice;
    }
    currencies[another].candlesticks[i].relative = re;
    if (re < currencies[another].lowestRelative) {
      currencies[another].lowestRelative = re;
    }
    if (currencies[another].highestRelative < re) {
      currencies[another].highestRelative = re;
    }
  }
}

void
Currency::GreyoutPrice()
{
  char buf[PRICEBUFSIZE], buf2[PRICEBUFSIZE];

  static int yoff = (1 < numScreens) ? tftHeight : 0; /// this is very temporal
  // To be modified
  
  itocsa(buf, PRICEBUFSIZE, prevPrice);
  ShowLastPrice(buf, pricePixel, TFT_DARKGREY, yoff); // make the price grey
  snprintf(buf2, PRICEBUFSIZE, "%.5f", relative);
  ShowRelativePrice(buf2, currencies[another].name, pricePixel, TFT_DARKGREY, yoff);
}

void
Currency::setAlert(class alert a)
{
  // today's high or low
  if (price < todayslow) {
    todayslow = price;

    snprintf(Alert.mesgbuf, MESGSIZE, "%s hit", name);
    Alert.setMesg1(Alert.mesgbuf);
    Alert.setMesg2("today's low");
    Alert.setBackColor(TFT_DOWNRED);
    Alert.setLastPrice(price);
    alertDuration = ALERT_DURATION;
  }
  else if (todayshigh < price) {
    todayshigh = price;

    snprintf(Alert.mesgbuf, MESGSIZE, "%s hit", name);
    Alert.setMesg1(Alert.mesgbuf);
    Alert.setMesg2("today's high");
    Alert.setBackColor(TFT_UPGREEN);
    Alert.setLastPrice(price);
    alertDuration = ALERT_DURATION;
  }

  // not event but set today's high and low properly
  if (todayshigh < candlesticks[numSticks - 1].highestPrice) {
    todayshigh = candlesticks[numSticks - 1].highestPrice;
  }
  if (candlesticks[numSticks - 1].lowestPrice < todayslow) {
    todayslow = candlesticks[numSticks - 1].lowestPrice;
  }

  // five minutes significant price change
  if (prevCandlestickTimestamp != candlesticks[numSticks - 1].timeStamp &&
	   FIVEMINUTES_THRESHOLD <=
	   abs((long)candlesticks[numSticks -1].startPrice - (long)candlesticks[numSticks -1].endPrice)
	   * 100 / (long)candlesticks[numSticks - 1].startPrice) {
    prevCandlestickTimestamp = candlesticks[numSticks - 1].timeStamp;
    if (candlesticks[numSticks - 1].startPrice < candlesticks[numSticks - 1].endPrice) {
      Alert.setBackColor(TFT_UPGREEN);
      snprintf(Alert.mesgbuf, MESGSIZE, "%s %.1f%% up within", name,
	       (float)(candlesticks[numSticks -1 ].endPrice - candlesticks[numSticks - 1].startPrice) * 100.0
	       / (float)candlesticks[numSticks - 1].startPrice);
    }
    else {
      Alert.setBackColor(TFT_DOWNRED);
      snprintf(Alert.mesgbuf, MESGSIZE, "%s %.1f%% down within", name,
	       (float)(candlesticks[numSticks -1 ].startPrice - candlesticks[numSticks - 1].endPrice) * 100.0
	       / (float)candlesticks[numSticks - 1].startPrice); 
    }
    Alert.setMesg1(Alert.mesgbuf);
    Alert.setMesg2("5 minutes.");
    Alert.setLastPrice(candlesticks[numSticks - 1].endPrice);
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
    Alert.setLastPrice(price);
    alertDuration = ALERT_DURATION;
  }
}

void
redrawChart(unsigned ind)
{
  LCD.fillScreen(TFT_BLACK);
  currencies[ind].ShowChart(0);
  if (1 < numScreens) {
    currencies[1 - ind].ShowChart(tftHeight);
  }
}

void
alertProc()
{
  if (0 < alertDuration) {
    Alert.flashAlert();
    alertDuration--;
  }
  if (alertDuration == 0) {
    timer.deleteTimer(Alert.alertId);
    redrawChart(cIndex);
  }
}

void
Currency::ShowCurrentPrice(bool forceReloadSticks)
{
  unsigned long t; // for current time
  unsigned long prevTime;
  char buf[PRICEBUFSIZE];

  if (0 < alertDuration) {
    return;
  }

  client.setCACert(bitbank_root_ca);

  Serial.println("\n==== Starting connection to server...");

  ShowUpdating(1 < numScreens ? tftHeight : 0);

  prevTime = prevTimeStamp;
  obtainLastPrice(&t);
  unsigned lastPriceOfOtherCurrency = currencies[another].obtainLastPrice(&t);
  prevRelative = relative;
  relative = (float)price / (float)lastPriceOfOtherCurrency;
  currencies[another].prevRelative = currencies[another].relative;
  currencies[another].relative = (float)lastPriceOfOtherCurrency / (float)price;
  itocsa(buf, PRICEBUFSIZE, price);
  Serial.print("last price = ");
  Serial.println(buf);
  // obtaining today's low and today's high
  if (forceReloadSticks ||
      /* (0 < price && */
      (0 == prevTime
	  || (minute(prevTimeStamp) % CANDLESTICK_WIDTH_MIN) < (minute(prevTime) % CANDLESTICK_WIDTH_MIN))) {
    // I forgot what '0 < price' means
    // Meaning of '(minute(prevTimeStamp) % CANDLESTICK_WIDTH_MIN) < (minute(prevTime) % CANDLESTICK_WIDTH_MIN)'
    // is that time exceeds specified CANDLESTICK_WIDTH_MIN, so that, it is necessary to obtain candlesticks.
    obtainSticks(numSticks, t);
    // get data for another currency
    currencies[another].obtainSticks(numSticks, t, candlesticks[numSticks - 1].timeStamp);
    calcRelative();
    // currencies[another].calcRelative();
  }

  SerialPrintTimestamp(candlesticks[numSticks - 1].timeStamp, TIMEZONE);

  // check events...
  // last event has the highest priority

  currencies[another].setAlert(Alert);
  setAlert(Alert);

  if (0 < alertDuration) {
    // Alert.setLastPrice(price);
    if (1 < numScreens || 0 < dedicatedPriceAreaHeight) {
      LCD.fillScreen(TFT_BLACK);
      ShowChart(0);
    }
    Alert.beginAlert();
    Alert.alertId = timer.setTimer(ALERT_INTERVAL, alertProc, ALERT_DURATION * (1000 / ALERT_INTERVAL) + 1);
  }
  else {
    // show the chart
    LCD.fillScreen(TFT_BLACK);
    ShowChart(0);
    if (1 < numScreens) {
      currencies[another].ShowChart(tftHeight);
    }
  }
}

void Currency::SwitchCurrency()
{
  Serial.println("Change triggered.");

  if (cIndex == 1 && MINIMUM_SPLITTABLE_HEIGHT < LCD.height()) {
    numScreens = 3 - numScreens; // (numScreens == 2) ? 1 : 2, numScreen = {1, 2}
    tftHeight = LCD.height() / numScreens;
    unsigned priceHeight = PriceFontHeight + LCD.fontHeight(OTHER_CURRENCY_BASE_VALUE_FONT);
    if (MINIMUM_SEPARATABLE_HEIGHT < tftHeight - priceHeight) {
      dedicatedPriceAreaHeight = priceHeight;
      tftHeight -= priceHeight;
    }
    else {
      dedicatedPriceAreaHeight = 0;
    }
    tftHalfHeight = tftHeight / 2;
  }

  cIndex = 1 - cIndex; // (cIndex == 1) ? 0 : 1

  redrawChart(cIndex);
}

void
_ShowCurrentPrice()
{
  if (WiFi.status() == WL_CONNECTED) {
    currencies[cIndex].ShowCurrentPrice(false);
  }
}

#define WIFI_ATTEMPT_LIMIT 30 // seconds for WiFi connection trial
static bool WiFiConnected = false;

void SecProc()
{
  static unsigned nWiFiTrial = 0;
  
  if (WiFi.status() == WL_CONNECTED) {
    if (!WiFiConnected) {
      WiFiConnected = true;
      Serial.println(" Connected");
      _ShowCurrentPrice();
    }
    if (changeTriggered) {
      changeTriggered = false;
      currencies[cIndex].SwitchCurrency();
    }
    if (rotationTriggered) {
      static const unsigned rotation_w[4] = {2, 3, 1, 0}; // for wide LCD
      static const unsigned rotation_n[4] = {1, 3, 1, 1}; // for narrow LCD
      rotationTriggered = false;
      unsigned r = LCD.getRotation();
      if (MINIMUM_WIDTH < LCD.height()) {
	r = rotation_w[r];
      }
      else {
	r = rotation_n[r];
      }
      unsigned prevNumSticks = numSticks;
      LCD.setRotation(r);
      tftHeight = LCD.height() / numScreens;
      tftWidth = LCD.width();
      tftHalfHeight = tftHeight / 2;
      numSticks =
	(tftWidth < MAX_HORIZONTAL_RESOLUTION) ? tftWidth / STICK_WIDTH : NUM_STICKS;
      if (numSticks != prevNumSticks) {
	currencies[cIndex].ShowCurrentPrice(true);
      }
      else {
	redrawChart(cIndex);
      }
    }
    if (currencyRotationTriggered) { // currency and screen rotation change triggered
      currencyRotationTriggered = false;
      static const unsigned cur_rot[4] = {1, 2, 3, 0};
      unsigned r = (LCD.getRotation() & 2); // 1 to 0, 3 to 1. no choice for 2 and 4
      unsigned cr = r + cIndex;
      cr = cur_rot[cr];
      if (r != cr / 2 + 1) {
	LCD.setRotation((cr & 2) + 1);
      }
      if (cIndex != cr % 2) {
	currencies[cIndex].SwitchCurrency();
      }
    }
  }
  else { // if FiFi.status() != WL_CONNECTED
    WiFiConnected = false;
    if (nWiFiTrial++ == 0) {
      if (0 < currencies[cIndex].price) { // connected before but lost
	// Grey out the price display
#if 0
	currencies[cIndex].GreyoutPrice();
	if (1 < numScreens) {
	  currencies[1 - cIndex].GreyoutPrice();
	}
#endif

#define CONNECTION_LOST "Reconnecting ..."
	// WiFi.begin(WIFIAP, WIFIPW);
	WiFi.reconnect();
	Serial.print("WiFi connection was lost.\nAttempting to reconnect to WiFi ");

	LCD.setTextColor(TFT_WHITE, TFT_BLUE);
	LCD.drawString(CONNECTION_LOST,
		       tftWidth / 2 - LCD.textWidth(CONNECTION_LOST, CONNECTINGFONT) / 2,
		       LCD.height() / 2 - LCD.fontHeight(CONNECTINGFONT) / 2, CONNECTINGFONT);
      }
      else { // not connected so far
	WiFi.begin(WIFIAP, WIFIPW);
	LCD.fillScreen(TFT_BLACK);
	delay(100);
	LCD.fillScreen(TFT_BLUE);
	LCD.setTextColor(TFT_WHITE);
	LCD.drawString("Connecting ...",
		       PADX, LCD.height() / 2 - LCD.fontHeight(CONNECTINGFONT) / 2, CONNECTINGFONT);
      }
    }
    if (WIFI_ATTEMPT_LIMIT < nWiFiTrial) {
      Serial.println(" Failed to coonect");
      WiFi.disconnect();
      nWiFiTrial = 0;
    }
    else {
      Serial.print(".");
    }
  }
}

#if !defined(ARDUINO_M5Stick_C) && !defined(ARDUINO_M5Stick_C_Plus) && !defined(ARDUINO_M5STACK_Core2)
void buttonEventProc()
{
  if (0 < alertDuration) {
    alertDuration = 0;
  }
  else {
    currencyRotationTriggered = true;
  }
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

#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus) || defined(ARDUINO_M5STACK_Core2)
#ifdef ARDUINO_M5STACK_Core2
  M5.Lcd.setRotation(1); // set it to 1 or 3 for landscape resolution
#else
  M5.Lcd.setRotation(1); // set it to 1 or 3 for landscape resolution
#endif
  tftHeight = M5.Lcd.height() / numScreens;
  tftWidth = M5.Lcd.width();
  M5.Lcd.fillScreen(BLACK);
#else
  tft.setRotation(1); // set it to 1 or 3 for landscape resolution
  tftHeight = tft.height() / numScreens;
  tftWidth = tft.width();
#endif

  tftHalfHeight = tftHeight / 2;
  numSticks =
    (tftWidth < MAX_HORIZONTAL_RESOLUTION) ? tftWidth / STICK_WIDTH : NUM_STICKS;

  currencies[0].another = 1;

  LCD.setTextPadding(PADX); // seems no effect by this line.
  LCD.setTextSize(1);
#if !(defined(ARDUINO_M5Stick_C) && !defined(ARDUINO_M5Stick_C_Plus))
  LCD.setFreeFont(PRICE_FONT); // Select a font for last price display
#endif
  PriceFontHeight = LCD.fontHeight(PRICEFONT) - PRICE_FONT_HEIGHT_ADJUSTMENT;

  unsigned priceHeight = PriceFontHeight + LCD.fontHeight(OTHER_CURRENCY_BASE_VALUE_FONT);
  if (MINIMUM_SEPARATABLE_HEIGHT < tftHeight - priceHeight) {
    dedicatedPriceAreaHeight = priceHeight;
    tftHeight -= priceHeight;
  }

  Serial.print("Attempting to connect to WiFi (");
  Serial.print(WIFIAP);
  Serial.print(") ");
  WiFi.begin(WIFIAP, WIFIPW);

  LCD.fillScreen(TFT_BLUE);
  LCD.setTextColor(TFT_WHITE);
  LCD.drawString("Connecting ...",
		 PADX, LCD.height() / 2 - LCD.fontHeight(CONNECTINGFONT) / 2, CONNECTINGFONT);

#if !defined(ARDUINO_M5Stick_C) && !defined(ARDUINO_M5Stick_C_Plus) && !defined(ARDUINO_M5STACK_Core2)
  attachInterrupt(digitalPinToInterrupt(BUTTON1), buttonEventProc, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON2), buttonEventProc, FALLING);
#endif
  
  timer.setInterval(1000, SecProc);
  timer.setInterval(60000, _ShowCurrentPrice);
}

void loop()
{
#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus) || defined(ARDUINO_M5STACK_Core2)
  M5.update();
  if (M5.BtnA.wasPressed()) {
    if (0 < alertDuration) {
      alertDuration = 0;
    }
    else {
      changeTriggered = true;
    }
  }
  if (M5.BtnB.wasPressed()) {
    if (0 < alertDuration) {
      alertDuration = 0;
    }
    else {
      rotationTriggered = true;
    }
  }
#endif
  timer.run();
}
