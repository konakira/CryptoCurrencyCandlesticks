#ifdef WOKWI_WEB
// --- PlatformIO env:wokwi build_flags ---
#define WOKWI
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define TFT_CS 15
#define TFT_BL 33
#define TFT_BACKLIGHT_ON HIGH
#define TFT_RGB_ORDER TFT_BGR
#define SPI_FREQUENCY 4000000
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_MISO 19
#define TFT_DC 2
#define TFT_RST 4
#define LGFX_USE_SPI 1
// ----------------------------------------
#endif

#include <Preferences.h>
Preferences pref;
#define PREFNAME "CCCSticks"

#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus) || defined(ARDUINO_M5STICK_S3) || defined(ARDUINO_M5STACK_Core2) || defined(ARDUINO_M5STACK_CORE_S3) || defined(ARDUINO_M5Stack_CoreInk)
#define ARDUINO_M5
#endif

#ifdef E_INK
unsigned long lastActivityMillis = 0;
#define USE_SPRITE
#endif // E_INK

#define LGFX_WHITE 0xFFFFFFU
#ifdef ARDUINO_M5
#include <M5Unified.h>
#ifdef ARDUINO_M5Stack_CoreInk
#define DEFAULT_ROTATION 0
#else // !ARDUINO_M5Stack_CoreInk
#define DEFAULT_ROTATION 1
#endif // !ARDUINO_M5Stack_CoreInk
#else // !ARDUINO_M5
#include <Arduino.h>
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device {
#ifdef WOKWI
  lgfx::Panel_ILI9341 _panel_instance; 
#else
  lgfx::Panel_ST7789  _panel_instance;
#endif
  lgfx::Bus_SPI      _bus_instance;
  lgfx::Light_PWM    _light_instance;

#if defined(R28T)
  lgfx::Touch_XPT2046 _touch_instance;
#endif
  
public:
  LGFX(void) {
    auto bus_cfg = _bus_instance.config();
    auto panel_cfg = _panel_instance.config();
    bus_cfg.spi_mode = 0;             
    bus_cfg.spi_3wire  = false;       
    bus_cfg.use_lock   = true;
    bus_cfg.dma_channel = SPI_DMA_CH_AUTO;

#ifdef ESPC6
    bus_cfg.freq_write = 40000000;
    bus_cfg.spi_host = SPI2_HOST;
    bus_cfg.pin_sclk = 7; bus_cfg.pin_mosi = 6; bus_cfg.pin_miso = -1; bus_cfg.pin_dc = 15;
#elif defined(TTGO)
    bus_cfg.freq_write = 40000000;
    bus_cfg.spi_host = VSPI_HOST;
    bus_cfg.pin_sclk = 18; bus_cfg.pin_mosi = 19; bus_cfg.pin_miso = -1; bus_cfg.pin_dc = 16;
#else // Standard ESP32 DevKit
    bus_cfg.freq_write = SPI_FREQUENCY;
#ifdef WOKWI
    bus_cfg.spi_host = VSPI_HOST;
#else
    bus_cfg.spi_host = HSPI_HOST;
#endif
    bus_cfg.pin_sclk = TFT_SCLK;
    bus_cfg.pin_mosi = TFT_MOSI;
    bus_cfg.pin_miso = TFT_MISO;
    bus_cfg.pin_dc = TFT_DC;
#endif    
    
    _bus_instance.config(bus_cfg);
    _panel_instance.setBus(&_bus_instance);

    // --- Panel Setup ---
#ifdef ESPC6
    panel_cfg.pin_cs = 14; panel_cfg.pin_rst = 21;
    panel_cfg.panel_width = 172; panel_cfg.panel_height = 320;
    panel_cfg.offset_x = 34; panel_cfg.invert = INVERT;
#elif defined(TTGO)
    panel_cfg.pin_cs = 5;
    panel_cfg.pin_rst = 23;
    // TTGO resolution is 135 x 240
    panel_cfg.panel_width  = 135; 
    panel_cfg.panel_height = 240;
    // configure offset
    panel_cfg.offset_x = 52; 
    panel_cfg.offset_y = 40; 
    panel_cfg.invert = INVERT;
#else // !ESPC6 && !TTGO
    panel_cfg.panel_width = TFT_WIDTH; panel_cfg.panel_height = TFT_HEIGHT;
    panel_cfg.offset_x = 0;
    panel_cfg.offset_y = 0;
    panel_cfg.pin_cs = TFT_CS;
    panel_cfg.pin_rst = TFT_RST;
#ifdef WOKWI
    panel_cfg.offset_rotation = 0;
    panel_cfg.dummy_read_pixel = 8;
    panel_cfg.dummy_read_bits = 1;
    panel_cfg.invert = INVERT;
    panel_cfg.readable = true;
#else
#ifdef INVERT
    panel_cfg.invert = INVERT;
#else
    panel_cfg.invert = true;
#endif
    panel_cfg.readable = false;
#endif // !WOKWI
#endif // !ESPC6 && !TTGO
    panel_cfg.rgb_order    = false;
    _panel_instance.config(panel_cfg);

    // --- Backlight Setup ---
    auto light_cfg = _light_instance.config();
    light_cfg.pin_bl = TFT_BL;
    light_cfg.pwm_channel = -1; 
    _light_instance.config(light_cfg);
    _panel_instance.setLight(&_light_instance);

    setPanel(&_panel_instance);
#if defined(R28T)
    // --- Touch Panel Setup (XPT2046) ---
    auto tcfg = _touch_instance.config();
    tcfg.x_min      = 300;
    tcfg.x_max      = 3900;
    tcfg.y_min      = 300;
    tcfg.y_max      = 3900;
    tcfg.pin_int    = 36;
    tcfg.bus_shared = false; 
    tcfg.offset_rotation = 0;

    tcfg.spi_host = VSPI_HOST; 
    tcfg.pin_sclk = 25;
    tcfg.pin_mosi = 32;
    tcfg.pin_miso = 39;
    tcfg.pin_cs   = 33;
    tcfg.freq     = 1000000;

    _touch_instance.config(tcfg);
    _panel_instance.setTouch(&_touch_instance);
#endif    
  }
};
LGFX tft;

#if !defined(TTGO) && !defined(ESPC6) // custom ESP32
#ifdef CDS
const int cds = CDS;
#endif
static bool backlight_is_on = true;
#define DEFAULT_ROTATION 0
#define BUTTON1 0 // GPIO0
#define TFT_BACKLIGHT_ON HIGH
#else // TTGO || ESPC6
#define DEFAULT_ROTATION 1
#ifdef ESPC6
#define BUTTON1 9 // 💡 ESP32-C6のBOOTボタンは「9番」！
#else // !ESPC6
#define BUTTON1 35 // GPIO35, not sure this works or not
#define BUTTON2 0 // GPIO0
#endif // !ESPC6
#endif // TTGO || ESPC6
#endif // !ARDUINO_M5

#ifdef WOKWI
#define ROTATION_OFFSET 4
#else
#define ROTATION_OFFSET 0
#endif

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <SPI.h>

#ifdef WOKWI
// --- SimpleTimer Implementation for WOKWI ---
  typedef void (*timer_callback)(void);
  class SimpleTimer {
  public:
    SimpleTimer() { for (int i = 0; i < 10; i++) enabled[i] = false; }
    void run() {
      for (int i = 0; i < 10; i++) {
        if (enabled[i] && (millis() - last_millis[i] >= (unsigned long)intervals[i])) {
          last_millis[i] = millis();
          if (counts[i] > 0) counts[i]--;
          if (counts[i] == 0) enabled[i] = false;
          (*callbacks[i])();
        }
      }
    }
    int setInterval(long d, timer_callback f) { return setTimer(d, f, -1); }
    int setTimer(long d, timer_callback f, int n) {
      for (int i = 0; i < 10; i++) {
        if (!enabled[i]) {
          intervals[i] = d; callbacks[i] = f; counts[i] = n;
          enabled[i] = true; last_millis[i] = millis();
          return i;
        }
      }
      return -1;
    }
    void deleteTimer(int id) { if (id >= 0 && id < 10) enabled[id] = false; }
  private:
    unsigned long last_millis[10];
    timer_callback callbacks[10];
    long intervals[10];
    int counts[10];
    bool enabled[10];
  };
#else
  #include <SimpleTimer.h>
#endif

#include <TimeLib.h>

#define MAX_HORIZONTAL_RESOLUTION 321

#ifdef ARDUINO_M5
#define PHYSICAL_LCD M5.Display
#else
#define PHYSICAL_LCD tft
#endif

#ifdef USE_SPRITE
// sprite canvas definitions
LGFX_Sprite canvas(&PHYSICAL_LCD);
#define LCD canvas  // Let all existing LCD.xxxx call to canvas
void
canvasPushSprite() {
  canvas.pushSprite(0, 0);
}
#else
#define LCD PHYSICAL_LCD
void canvasPushSprite() {}
#endif

// color definitions
#ifdef E_INK
#define COLOR_BG      TFT_WHITE
#define COLOR_CONNECTIONBG TFT_WHITE
#define COLOR_TEXT    TFT_BLACK
#define COLOR_UP      TFT_WHITE
#define COLOR_DOWN    TFT_BLACK
#define COLOR_WICK    TFT_BLACK
#define COLOR_GRID    COLOR_TEXT
#define COLOR_PRICE_L TFT_BLACK
#define COLOR_TIME TFT_BLACK
#define COLOR_BATTERY TFT_BLACK
#define COLOR_CHARGING COLOR_TEXT
#else
#define COLOR_BG      TFT_BLACK
#define COLOR_CONNECTIONBG TFT_BLUE
#define COLOR_TEXT    LGFX_WHITE
#define COLOR_UP      TFT_UPGREEN
#define COLOR_DOWN    TFT_DOWNRED
#define COLOR_WICK    TFT_LIGHTGREY
#define COLOR_GRID    TFT_DARKBLUE
#define COLOR_PRICE_L (priceColor == TFT_GREEN ? TFT_UPGREEN : TFT_DOWNRED)
#define COLOR_TIME TFT_LIGHTGREY
#define COLOR_BATTERY TFT_WHITE
#define COLOR_CHARGING TFT_UPGREEN
#endif

// WiFi AP information should be stored at auth.h
struct WifiCredential {
  const char* ssid;
  const char* pass;
};

#ifdef WOKWI
struct WifiCredential wifi_list[] = {{"Wokwi-GUEST", ""}};
#else
#include "auth.h" // use the above WifiCredential to list multiple APs as wifi_list in "auth.h"
#endif
const int wifi_count = sizeof(wifi_list) / sizeof(wifi_list[0]);

WiFiClientSecure client;
WiFiMulti wifiMulti;

SimpleTimer timer;

#ifndef BRIGHTNESS
#define BRIGHTNESS 255
#endif

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

#ifndef E_INK
#define CANDLESTICK_WIDTH "5min"
#define CANDLESTICK_WIDTH_MIN 5
#define TIME_LABEL_INTERVAL 3 // 3 hours
#else // E_INK
#define CANDLESTICK_WIDTH "1day"
#define CANDLESTICK_WIDTH_MIN 1440 // 24 * 60
#define TIME_LABEL_INTERVAL 10 // 10 days
#endif // E_INK

#ifdef E_INK
#define STICK_WIDTH 5 // width of a candle stick
#define COLOR_BORDER COLOR_TEXT
#define ETH_HLINE_UNIT 20000
#define BTC_HLINE_UNIT 500000
#else
#define STICK_WIDTH 3 // width of a candle stick
#define ETH_HLINE_UNIT 5000
#define BTC_HLINE_UNIT 100000
#endif
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
  bool obtainSticks(unsigned n, unsigned long t);
  bool obtainSticks(unsigned n, unsigned long t, unsigned long lastTimeStamp);
  void sendRequest(unsigned long t);
  unsigned readHeader();
  void calcRelative();
  void ShowChart(int yoff);
  void ShowCurrentPrice(bool forceReloadSticks);
  void SwitchCurrency();
  void ShowCurrencyName(const char *buf, int yoff);
  void ShowStatus(const char *status, int yoff);
  void ShowUpdating(int yoff);
#ifndef E_INK
  void setAlert(class alert a);
#endif
} currencies[2] = {{"ETH", "eth_jpy", ETH_HLINE_UNIT}, {"BTC", "btc_jpy", BTC_HLINE_UNIT}};

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
#define MINIMUM_SEPARATABLE_HEIGHT 120 // for price and chart seperation
#define MINIMUM_WIDTH 199 // allowed minimum width used in rotation

void
Currency::sendRequest(unsigned long t)
{
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
#ifndef E_INK
    sprintf(yyyymmdd, "%04d%02d%02d", year(t), month(t), day(t));
#else // E_INK
    sprintf(yyyymmdd, "%04d", year(t));
#endif // E_INK
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
}

unsigned
Currency::readHeader()
{
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
  return errcounter;
}

bool
Currency::obtainSticks(unsigned n, unsigned long t, unsigned long lastTimeStamp)
{
  unsigned errcounter = 0;

  // Serial.println("\nobtainSticks called.");
  while (0 < n) {
    if (client.connect(SERVER, 443)) {
      Serial.println("\nConnected to http server for sticks.");

// #define SHOW_HTTPHEADERS

      // HTTP request:
      sendRequest(t);
      errcounter = readHeader();

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
	    unsigned stickOffset, ohlcvOffset, nloops;
	    if (n <= nSticks) { // enough sticks obtained
	      Serial.println(" (enough)");
	      stickOffset = 0;
	      ohlcvOffset = nSticks - n;
	      nloops = n;
	    }
	    else {
	      Serial.println(" (not enough)");
	      stickOffset = n - nSticks;
	      ohlcvOffset = 0;
	      nloops = nSticks;
	    }
	    for (unsigned i = 0 ; i < nloops ; i++) {
	      unsigned stickIndex = i + stickOffset;
	      unsigned ohlcvIndex = i + ohlcvOffset;

	      // copy the last n data from JSON
	      candlesticks[stickIndex].startPrice = ohlcv[ohlcvIndex][0].as<unsigned>();
	      candlesticks[stickIndex].highestPrice = ohlcv[ohlcvIndex][1].as<unsigned>();
	      candlesticks[stickIndex].lowestPrice = ohlcv[ohlcvIndex][2].as<unsigned>();
	      candlesticks[stickIndex].endPrice = ohlcv[ohlcvIndex][3].as<unsigned>();
	      candlesticks[stickIndex].timeStamp =
		(unsigned long)(ohlcv[ohlcvIndex][5].as<unsigned long long>() / 1000);
	    }
	    if (n <= nSticks) {
	      n = 0;

	      if (todayshigh == 0) { // if 'todayshigh' is not set
		// set 'todayshigh' based on candlestick information
		// actual 'todayshigh' should be set using last price, out of this routine
		unsigned today = day(candlesticks[numSticks - 1].timeStamp + TIMEZONE);

		if (day(candlesticks[0].timeStamp + TIMEZONE) == today) {
		  unsigned long ts;
		  bool needMoreSticks = false;
		  // have to check the data before the first item in candlesticks[]
		  // This is in order to obtain the real "today's high" and "low".
		  // Note that this does not work in the case that 9:00am is out of holizontal range
		  // This should be fixed in the future version.
		  ts = (unsigned long)(ohlcv[0][5].as<unsigned long long>() / 1000);
		  if (day(ts + TIMEZONE) == today) {
		    // Not enough data to obtain today's high and low
		    // This should be programmed in the future
		    needMoreSticks = true;
		  }
		  
		  for (unsigned i = 0 ; i < nSticks - n ; i++) {
                    unsigned h, l;
                    
                    // get necessary data from JSON
		    h = ohlcv[i][1].as<unsigned>();
		    l = ohlcv[i][2].as<unsigned>();
		    ts = (unsigned long)(ohlcv[i][5].as<unsigned long long>() / 1000);
		    if (day(ts + TIMEZONE) == today) {
		      if (todayshigh == 0) {
			todayshigh = h;
			todayslow = l;
		      }
		      else if (todayshigh < h) {
			todayshigh = h;
		      }
		      else if (l < todayslow) {
			todayslow = l;
		      }
                    }
                  }
                  if (needMoreSticks) {
                    // Should be programmed here in the future
                    t -= 24 * 60 * 60; // for data one day before
                    // HTTP request:
                    sendRequest(t);
                    errcounter = readHeader();

                  }
                }
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

	    }
	    else {
	      n -= nSticks; // to fill remaining slots
	      t -= 24 * 60 * 60; // for data one day before
	    }
	  }
	  else { // not success
	    Serial.println(F("deserializeJson() succeded w/ false 'success' flag."));
	    return false;
	  }
	}
	else {
	  Serial.print(F("deserializeJson() failed: "));
	  Serial.println(error.f_str());
	  return false;
	}
      }
      else {
	Serial.println(F("http read timedout."));
	return false;
      }
    }
    else {
      Serial.println("\nConnection failed!");
      return false;
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
  Serial.print("\ntoday = ");
  Serial.print(todayslow);
  Serial.print(" - ");
  Serial.println(todayshigh);
  Serial.print("chart = ");
  Serial.print(lowest);
  Serial.print(" - ");
  Serial.println(highest);
  if (todayslow < lowest) {
    lowest = todayslow;
  }
  if (highest < todayshigh) {
    highest = todayshigh;
  }
  return true;
}

bool
Currency::obtainSticks(unsigned n, unsigned long t)
{
  return obtainSticks(n, t, 0);
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
    StaticJsonDocument<512> doc;

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
	price = doc["data"]["last"].as<unsigned>();
      }
    }
    client.stop();
  }
  if (t) {
    *t = prevTimeStamp;
  }
  return price;
}

#ifndef E_INK
#define ALERT_INTERVAL 500 // msec    
#define ALERT_DURATION (30 /* sec */ * (1000 / ALERT_INTERVAL)) // times ALERT_INTERVAL
#endif
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

#define FONTN2 &fonts::Font2
#define FONTN4 &fonts::Font4
#define CONNECTINGFONT FONTN4
#define PRICE_FONT_HEIGHT_ADJUSTMENT -5
#define OTHER_FONT_WIDTH_ADJUSTMENT 2

#if defined(ARDUINO_M5Stick_C) && !defined(ARDUINO_M5Stick_C_Plus) // just for M5 Stick C
#define PRICEFONT FONTN4
#define OTHER_CURRENCY_BASE_VALUE_FONT &fonts::FreeSans9pt7b
#define BASE_DIFF 0 // base difference between relative price font and its unit font
#else // Other than M5 Stick C
#ifdef ARDUINO_M5Stack_CoreInk
#define PRICEFONT &fonts::FreeSerifBold18pt7b
#else
#define PRICEFONT &fonts::FreeSerifBold24pt7b
#endif
#define OTHER_CURRENCY_BASE_VALUE_FONT FONTN4
#define BASE_DIFF 4 // base difference between relative price font and its unit font
#endif // Other than M5 Stick C

#define TFT_DOWNRED  0xFF0000U
#define TFT_UPGREEN  0x00FF00U
#define TFT_DARKBLUE 0x000088U

#define ONEMINUTE_THRESHOLD 1 // per cent
#define FIVEMINUTES_THRESHOLD 1 // per cent
#define PADX 5
#define PADY 5
#define MESGSIZE 64
#define ALERT_BLACK_DURATION 200 // msec

#ifdef E_INK
#define MONITOR_DEEPSLEEP_TEST 1 // 1 to show time, 0 to show date

void ShowHeaderDate(unsigned yoff) {
  char buf[16];
  // Get time from prevTimeStamp which is updated by ticker
  unsigned long t = currencies[cIndex].prevTimeStamp + TIMEZONE;
#if (0 < MONITOR_DEEPSLEEP_TEST)
  sprintf(buf, "%d:%02d", hour(t), minute(t));
#else
    sprintf(buf, "%d/%d", month(t), day(t));
#endif
  LCD.setTextColor(COLOR_TEXT);
  // Place it to the left of the battery area
  int x = tftWidth - LCD.textWidth(buf, OTHER_CURRENCY_BASE_VALUE_FONT);
  LCD.drawString(buf, (x < 0 ? 5 : x), yoff, OTHER_CURRENCY_BASE_VALUE_FONT);
}
#else
void ShowHeaderDate(unsigned yoff) {}
#endif

void
DrawStringWithShade(const char *buf, int x, int y, const lgfx::v1::IFont* font, int color, int shade)
{
#ifndef E_INK
  // Disable text shade on Core Ink to prevent the "ghosting" or "double letters" smudge
  LCD.setTextColor(COLOR_BG);
  LCD.drawString(buf, x - shade, y - shade, font);
  LCD.drawString(buf, x + shade, y + shade, font);
  LCD.setTextColor(color);
#else
  LCD.setTextColor(color, COLOR_BG);
#endif
  LCD.drawString(buf, x, y, font);
}

#define BAT_POS_TOPRIGHT 1
#define BAT_POS_TOPLEFT 2
#define BAT_POS_BOTTOMLEFT 3
#ifdef ARDUINO_M5
#define TOP_OFFSET 1 // per 10
#define HEIGHT_RANGE 8 // per 10

void
ShowBatteryStatus(unsigned position)
{
  unsigned batstat = M5.Power.getBatteryLevel(); // Just calling a function to obtain battery level.
  bool charging = M5.Power.isCharging();
  
  char buf[6];
  sprintf(buf, "%d%%", batstat);
  unsigned fHeight = LCD.fontHeight(FONTN2);
  unsigned batxoff = 1;
  unsigned batyoff = ((fHeight - 2) * TOP_OFFSET / 10) + 1;
  unsigned batheight = (fHeight - 2) * HEIGHT_RANGE / 10;
  unsigned batwidth = (batheight - 2) * 2 - 2;
  unsigned pluslen = batheight / 3;

  unsigned top, bottom, left, right;

  switch (position) {
  case BAT_POS_TOPLEFT:
    top = batyoff;
    left = batxoff;
    break;
  case BAT_POS_BOTTOMLEFT:
    top = LCD.height() - fHeight + batyoff;
    left = batxoff;
    break;
  case BAT_POS_TOPRIGHT:
  default:
    top = batyoff;
    left = LCD.width() - batwidth - LCD.textWidth("100%", FONTN2);
    break;
  }
  bottom = top + batheight;
  right = left + batwidth;
    
  LCD.fillRect(left - 1, top - 1, right - left + 3, bottom - top + 2, COLOR_BG);
  LCD.drawLine(left + 1, top, right - 3, top, COLOR_BATTERY);
  LCD.drawLine(left + 1, bottom, right - 3, bottom, COLOR_BATTERY);
  LCD.drawLine(left, top + 1, left, bottom - 1, COLOR_BATTERY);
  LCD.drawLine(right - 2, top + 1, right - 2, bottom - 1, COLOR_BATTERY);
  LCD.fillRect(right, top + (bottom - top - pluslen) / 2, 2, pluslen, COLOR_BATTERY);
  LCD.fillRect(left + 2, top + 2,
	       (right - left - 4) * batstat / 100, bottom - top - 3,
	       charging ? COLOR_CHARGING : COLOR_TEXT);

  DrawStringWithShade(buf, right + 5, top - batyoff, FONTN2, COLOR_TEXT, 1);
}
#else
void
ShowBatteryStatus(unsigned position) {}
#endif

void
ShowLastPrice(char *buf, int lastPricePixel, unsigned priceColor, int yoff)
{
  int textY;
  if (dedicatedPriceAreaHeight) {
    textY = 0;
    if (yoff == 0) {
      ShowBatteryStatus(BAT_POS_TOPRIGHT);
    }
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
    if (yoff == 0) {
      ShowBatteryStatus(LCD.fontHeight(FONTN2) < textY ? BAT_POS_TOPLEFT : BAT_POS_BOTTOMLEFT);
    }
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
		      PADX + LCD.textWidth(buf, OTHER_CURRENCY_BASE_VALUE_FONT) + OTHER_FONT_WIDTH_ADJUSTMENT,
		      textY + yoff + LCD.fontHeight(OTHER_CURRENCY_BASE_VALUE_FONT) - LCD.fontHeight(FONTN2) - BASE_DIFF, FONTN2, priceColor, BORDER_WIDTH);
}

void
Currency::ShowCurrencyName(const char *buf, int yoff)
{
  int textY;
  if (pricePixel < tftHalfHeight) {
    textY = tftHeight - LCD.fontHeight(FONTN2) * 2;
  }
  else {
    textY = LCD.fontHeight(FONTN2);
  }
#ifdef E_INK
  LCD.setTextColor(COLOR_TEXT, COLOR_BG);
#else
  LCD.setTextColor(COLOR_TEXT);
#endif
  LCD.drawString(buf, tftWidth - LCD.textWidth(buf, FONTN2) - 1, textY + yoff, FONTN2);
}

static const char *updating = "Updating...";

// I tried to add this ShowStatus() function which generalized ShowUpdating(),
// for showing connection failed status, but not used yet.

void
Currency::ShowStatus(const char *status, int yoff)
{
  int textY;
  if (pricePixel < tftHalfHeight) {
    textY = tftHeight - LCD.fontHeight(FONTN2) * 2 + yoff;
  }
  else {
    textY = LCD.fontHeight(FONTN2);
  }
  LCD.setTextColor(COLOR_TEXT, COLOR_CONNECTIONBG);
  LCD.drawString(status, tftWidth - LCD.textWidth(status, FONTN2) - 1,
		 textY + dedicatedPriceAreaHeight, FONTN2);
  canvasPushSprite();
}

void
Currency::ShowUpdating(int yoff)
{
  ShowStatus(updating, yoff);
}

#ifndef E_INK
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
    LCD.setTextColor(LGFX_WHITE);
    itocsa(buf, PRICEBUFSIZE, lastPrice);
    textColor = LGFX_WHITE;
    showAlert();
  }
  void flashAlert() {
    textColor = (textColor == TFT_BLACK) ? LGFX_WHITE : TFT_BLACK;
    LCD.setTextColor(textColor);
    showAlert();
  }
private:
  void showAlert() {
    unsigned yoff = 1 < numScreens ? tftHeight : 0 + dedicatedPriceAreaHeight;

    LCD.drawString(alertmesg1, PADX, PADY + yoff, FONTN4);
    LCD.drawString(alertmesg2, PADX, LCD.fontHeight(FONTN4) + PADY + yoff, FONTN4);
    LCD.drawString(buf,
		   tftWidth / 2 - LCD.textWidth(buf, PRICEFONT) / 2,
		   tftHeight - PriceFontHeight + yoff, PRICEFONT);  
    canvasPushSprite();
  }
  char buf[PRICEBUFSIZE];
  const char *alertmesg1;
  const char *alertmesg2;
  unsigned textColor = LGFX_WHITE;
  unsigned alertbgcolor = TFT_DOWNRED; // alert color by default
  unsigned lastPrice;
} Alert;
#endif // !E_INK

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
  unsigned stickColor = COLOR_DOWN, priceColor = TFT_GREEN;

  // show the chart
  
  yoff += dedicatedPriceAreaHeight;

#define LOW_PRICE_TEST 0 // 1 for rare case testing, 0 for no testing
#if LOW_PRICE_TEST
  price = lowest;
#endif
  
  if (highest < price) {
    highest = price;
  }
  else if (price < lowest) {
    lowest = price;
  }
  if (lowest == highest) {
    return;
  }
  
  // draw horizontal price lines
  if (price < prevPrice) {
    priceColor = TFT_RED;
  }
  for (int i = lowest / priceline + 1 ; i * priceline < highest ; i++) {
    int y = map(i * priceline, lowest, highest, tftHeight, 0);
#ifdef E_INK
    // Draw explicit dotted line for e-ink
    for (int x = 0; x < tftWidth; x += 4) LCD.drawPixel(x, y + yoff, COLOR_GRID);
#else
    LCD.drawFastHLine(0, y + yoff, tftWidth, COLOR_GRID);
#endif
  }

  // get the position to draw last price
  pricePixel = map(price, lowest, highest, tftHeight, 0);

  // get initial position for relative price
  unsigned prevRel = floatmap(candlesticks[0].relative, lowestRelative, highestRelative, tftHeight, 0);

#ifndef E_INK
  unsigned prevTimeVal = hour(candlesticks[0].timeStamp + TIMEZONE);
#else
  unsigned prevTimeVal = day(candlesticks[0].timeStamp + TIMEZONE);
#endif
    
  itocsa(buf, PRICEBUFSIZE, highest);

  // draw candlesticks
  for (unsigned i = 0 ; i < numSticks ; i++) {
    // draw vertical grid line (Hour or Day boundary)
#ifndef E_INK
    unsigned curTimeVal = hour(candlesticks[i].timeStamp + TIMEZONE);
#else
    unsigned curTimeVal = day(candlesticks[i].timeStamp + TIMEZONE);
#endif

    if (curTimeVal != prevTimeVal) {
      prevTimeVal = curTimeVal;
#ifndef E_INK
      LCD.drawFastVLine(i * STICK_WIDTH + 1, yoff, tftHeight, COLOR_GRID);
#endif
      if (curTimeVal % TIME_LABEL_INTERVAL == 0) {
	char bufLabel[8];
#ifndef E_INK
	snprintf(bufLabel, sizeof(bufLabel), "%d", curTimeVal);
#else
	// Show Month/Day for E-Ink daily chart
	snprintf(bufLabel, sizeof(bufLabel), "%d/%d", month(candlesticks[i].timeStamp + TIMEZONE), curTimeVal);
        // Only draw dotted grid for e-ink on label intervals (every 10 days)
        for (int y = yoff; y < tftHeight + yoff; y += 4) LCD.drawPixel(i * STICK_WIDTH + 1, y, COLOR_GRID);
#endif
	int offset = LCD.textWidth(bufLabel, FONTN2) / 2;
	if (i * STICK_WIDTH < tftWidth - LCD.textWidth(buf, FONTN2) - offset - PADX) {
	  // if we have enough space around vertical line, draw the time
	  unsigned textY = 0;

	  if (pricePixel < tftHalfHeight) {
	    textY = tftHeight - LCD.fontHeight(FONTN2);
	  }
	  LCD.setTextColor(COLOR_TIME);
	  LCD.drawString(bufLabel, i * STICK_WIDTH - offset, textY + yoff, FONTN2);
	}
      }
    }

#ifndef E_INK    
    if (0 < i) { // draw graph for relative prices
      unsigned curRel = floatmap(candlesticks[i].relative, lowestRelative, highestRelative,
				 tftHeight- 1, 2);
      // note that the lowest pixel is 2, instead of 0 to prevent collision in
      // dual screen mode
      LCD.drawLine(i * STICK_WIDTH - 2, prevRel + yoff, i * STICK_WIDTH + 1, curRel + yoff, TFT_ORANGE);
      prevRel = curRel;
    }
#endif

    // draw candlesticks
    int lowestPixel, highestPixel, lowPixel, pixelHeight;
    
    lowestPixel = map(candlesticks[i].lowestPrice, lowest, highest, tftHeight, 0);
    highestPixel = map(candlesticks[i].highestPrice, lowest, highest, tftHeight, 0);

    if (candlesticks[i].startPrice < candlesticks[i].endPrice) {
      lowPixel = map(candlesticks[i].endPrice, lowest, highest, tftHeight, 0);
      pixelHeight = map(candlesticks[i].startPrice, lowest, highest, tftHeight, 0)
	- lowPixel;
      stickColor = COLOR_UP;
    }
    else {
      lowPixel = map(candlesticks[i].startPrice, lowest, highest, tftHeight, 0);
      pixelHeight = map(candlesticks[i].endPrice, lowest, highest, tftHeight, 0)
	- lowPixel;
      stickColor = COLOR_DOWN;
    }

    LCD.drawFastVLine(i * STICK_WIDTH + 1, highestPixel + yoff, lowestPixel - highestPixel, COLOR_WICK);
    LCD.fillRect(i * STICK_WIDTH, lowPixel + yoff, STICK_WIDTH, pixelHeight, stickColor);
#ifdef COLOR_BORDER
    LCD.drawRect(i * STICK_WIDTH, lowPixel + yoff, STICK_WIDTH, pixelHeight, COLOR_BORDER);
#endif
  }

  // draw price horizontal line
  LCD.drawFastHLine(0, pricePixel + yoff, tftWidth, COLOR_PRICE_L);
 
  // draw highest and lowest price in the chart
  itocsa(buf, PRICEBUFSIZE, highest);
  DrawStringWithShade(buf, tftWidth - LCD.textWidth(buf, FONTN2) - 1, yoff, FONTN2, COLOR_TEXT, 1);
 
  itocsa(buf, PRICEBUFSIZE, lowest);

  DrawStringWithShade(buf, tftWidth - LCD.textWidth(buf, FONTN2) - 1,
  		      tftHeight + yoff - LCD.fontHeight(FONTN2), FONTN2, COLOR_TEXT, 1);
  // show currency name
  ShowCurrencyName(name, yoff);
  
  // draw last price
  itocsa(buf, PRICEBUFSIZE, price);

#ifdef E_INK
  priceColor = COLOR_TEXT; // solid color for e-Ink
#endif
  // show the current cryptocurrency price on TTGO-T-display
  yoff -= dedicatedPriceAreaHeight;
  ShowLastPrice(buf, pricePixel, priceColor, yoff);
  snprintf(buf2, PRICEBUFSIZE, "%.5f", relative);
#ifndef E_INK
  priceColor = (relative < prevRelative ? TFT_RED : TFT_GREEN);
#endif
  ShowRelativePrice(buf2, currencies[another].name, pricePixel, priceColor, yoff);
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

#define FLASH_TEST 0 // 1 for rare case testing, 0 for no testing

#ifndef E_INK
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
  else if (todayshigh < price || FLASH_TEST) {
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
#endif // !E_INK

void
redrawChart(unsigned ind)
{
  LCD.fillScreen(COLOR_BG);
  currencies[ind].ShowChart(0);
  if (1 < numScreens) {
    currencies[1 - ind].ShowChart(tftHeight);
  }
  canvasPushSprite();
}

#ifndef E_INK
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
#endif // !E_INK

void
Currency::ShowCurrentPrice(bool forceReloadSticks)
{
  unsigned long t; // for current time
  unsigned long prevTime;
  char buf[PRICEBUFSIZE];

#ifndef E_INK
  if (0 < alertDuration) {
    return;
  }
#endif

  client.setInsecure();

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

#define NRETRY 20
    
    for (int i = 0 ; i < NRETRY ; i++) {
      if (obtainSticks(numSticks, t)) break;
      delay(500);
    }

    // get data for another currency
    for (int i = 0 ; i < NRETRY ; i++) {
      if (currencies[another].obtainSticks(numSticks, t, candlesticks[numSticks - 1].timeStamp)) break;
      delay(500);
    }
    calcRelative();
    // currencies[another].calcRelative();
  }

  SerialPrintTimestamp(candlesticks[numSticks - 1].timeStamp, TIMEZONE);

  // check events...
  // last event has the highest priority

#ifndef E_INK
  currencies[another].setAlert(Alert);
  setAlert(Alert);

  if (0 < alertDuration) {
    // Alert.setLastPrice(price);
    if (1 < numScreens || 0 < dedicatedPriceAreaHeight) {
      LCD.fillScreen(COLOR_BG);
      ShowChart(0);
    }
    Alert.beginAlert();
    canvasPushSprite();
    Alert.alertId = timer.setTimer(ALERT_INTERVAL, alertProc, ALERT_DURATION * (1000 / ALERT_INTERVAL) + 1);
  }
  else {
#endif// !E_INK
    // show the chart
    LCD.fillScreen(COLOR_BG);
    ShowChart(0);
    if (1 < numScreens) {
      currencies[another].ShowChart(tftHeight);
    }
    canvasPushSprite();
#ifndef E_INK
  }
#endif
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

#ifdef E_INK
void
UpdateIdleTimer()
{
  lastActivityMillis = millis();
}
void
UpdateIdleTimerIfNotSet()
{
  if (lastActivityMillis == 0) {
    UpdateIdleTimer();
  }
}
#else
void UpdateIdleTimer() {}
void UpdateIdleTimerIfNotSet() {}
#endif

void
_ShowCurrentPrice()
{
  if (WiFi.status() == WL_CONNECTED) {
    currencies[cIndex].ShowCurrentPrice(false);
  }
  UpdateIdleTimerIfNotSet();
}

#define CINDEX "cIndex"
#define NUMSCREENS "numScreens"
#define ROTATION "rotation"

void saveSettings()
{
  if (pref.begin(PREFNAME, false)) {
    pref.putUInt(CINDEX, cIndex);
    pref.putUInt(NUMSCREENS, numScreens);
    pref.putUInt(ROTATION, LCD.getRotation());
    pref.end();
  }
  Serial.printf("numScreens = %d\n", numScreens);
}

void restoreSettings()
{
  if (pref.begin(PREFNAME, false)) {
    cIndex = pref.getUInt(CINDEX, 0);
    numScreens = pref.getUInt(NUMSCREENS, 1);
    Serial.printf("numScreens = %d\n", numScreens);
    unsigned r = pref.getUInt(ROTATION, 255);
    pref.end();
    if (r != 255) {
      LCD.setRotation(r);
    }
  }
}

#define WIFI_ATTEMPT_LIMIT 30 // seconds for WiFi connection trial
static bool WiFiConnected = false;

void SecProc()
{
  static unsigned nWiFiTrial = 0;

#ifdef CDS
  // control LED Backlight
  unsigned br = analogRead(cds);
  if (br < 1 && backlight_is_on) {
    tft.setBrightness(0);
    backlight_is_on = false;
    Serial.println("Back light turned off");
  }
  else if (0 < br && !backlight_is_on) {
    tft.setBrightness(BRIGHTNESS);
    backlight_is_on = true;
    Serial.print("Back light turned on. CdS = ");
    Serial.println(br);
  }
#endif
  
  if (WiFi.status() == WL_CONNECTED) {
    if (!WiFiConnected) {
      WiFiConnected = true;
      Serial.println(" Connected");
      _ShowCurrentPrice();
    }
    if (changeTriggered) {
      changeTriggered = false;
      currencies[cIndex].SwitchCurrency();
      UpdateIdleTimer();
      saveSettings();
    }
    if (rotationTriggered) {
      static const unsigned rotation_w[4] = {2, 3, 1, 0}; // for wide LCD
      static const unsigned rotation_n[4] = {2, 3, 0, 1}; // for narrow LCD
      rotationTriggered = false;
      unsigned r = LCD.getRotation();
      // Calculate the shortest side to tell if it can rotate 90 degrees
      unsigned short_side = (LCD.width() < LCD.height()) ? LCD.width() : LCD.height();
      if (MINIMUM_WIDTH < short_side) {
	r = rotation_w[r];
      }
      else {
	r = rotation_n[r];
      }
      unsigned prevNumSticks = numSticks;
      LCD.setRotation(r + ROTATION_OFFSET);
      tftHeight = LCD.height() / numScreens;
      tftWidth = LCD.width();
      tftHalfHeight = tftHeight / 2;

      unsigned priceHeight = PriceFontHeight + LCD.fontHeight(OTHER_CURRENCY_BASE_VALUE_FONT);
      if (MINIMUM_SEPARATABLE_HEIGHT < tftHeight - priceHeight) {
        dedicatedPriceAreaHeight = priceHeight;
        tftHeight -= priceHeight;
      }
      else {
        dedicatedPriceAreaHeight = 0;
      }

      numSticks =
	(tftWidth < MAX_HORIZONTAL_RESOLUTION) ? tftWidth / STICK_WIDTH : NUM_STICKS;
      if (numSticks != prevNumSticks) {
	currencies[cIndex].ShowCurrentPrice(true);
      }
      else {
	redrawChart(cIndex);
      }
      UpdateIdleTimer();
      saveSettings();
    }
    if (currencyRotationTriggered) { // currency and screen rotation change triggered
      currencyRotationTriggered = false;
#if defined(TTGO) || defined(ESPC6)	
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
#else
      currencies[cIndex].SwitchCurrency();
#endif
      UpdateIdleTimer();
      saveSettings();
    }
  }
  else { // if FiFi.status() != WL_CONNECTED
    WiFiConnected = false;
    if (nWiFiTrial++ == 0) {
      if (0 < currencies[cIndex].price) { // connected before but lost
	// Grey out the price display

#define CONNECTION_LOST "Reconnecting ..."
	wifiMulti.run();
	Serial.print("WiFi connection was lost.\nAttempting to reconnect to WiFi ");

	LCD.setTextColor(LGFX_WHITE, COLOR_CONNECTIONBG);
	LCD.drawString(CONNECTION_LOST,
		       tftWidth / 2 - LCD.textWidth(CONNECTION_LOST, CONNECTINGFONT) / 2,
		       LCD.height() / 2 - LCD.fontHeight(CONNECTINGFONT) / 2, CONNECTINGFONT);
      }
      else { // not connected so far
	wifiMulti.run();
	LCD.fillScreen(COLOR_CONNECTIONBG); // Clear canvas with proper background color
	LCD.setTextColor(COLOR_TEXT);
	LCD.drawString("Connecting ...",
		       PADX, LCD.height() / 2 - LCD.fontHeight(CONNECTINGFONT) / 2, CONNECTINGFONT);
      }
      canvasPushSprite();
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
#ifdef E_INK  
#define IDLE_TIME_TO_SLEEP (60 * 1000) // msec

  if (millis() - lastActivityMillis > IDLE_TIME_TO_SLEEP) {
    unsigned long currentJST = currencies[1 - cIndex].prevTimeStamp + TIMEZONE;

    ShowHeaderDate(LCD.fontHeight(PRICEFONT));
    canvas.pushSprite(0, 0);
    M5.Display.waitDisplay();

    Serial.printf("cIndex = %d, numScreens = %d, rotation = %d\n", cIndex, numScreens, LCD.getRotation());
    Serial.flush();

    delay(2000);

    {
      m5::rtc_time_t rtc_time;
      rtc_time.hours   = hour(currentJST);
      rtc_time.minutes = minute(currentJST);
      rtc_time.seconds = second(currentJST);
      M5.Rtc.setTime(&rtc_time);

      m5::rtc_date_t rtc_date;
      rtc_date.year  = year(currentJST);
      rtc_date.month = month(currentJST);
      rtc_date.date  = day(currentJST);
      M5.Rtc.setDate(&rtc_date);

      m5::rtc_time_t alarm_time;
      alarm_time.hours   = 0;
      alarm_time.minutes = 0;
      alarm_time.seconds = 0;
      M5.Rtc.setAlarmIRQ(alarm_time);

      M5.Power.powerOff();
    }
    // M5.Power.deepSleep((unsigned long long)secondsToSleep * 1000000UL, false);
    // M5.Power.timerSleep(secondsToSleep);
  }
#endif  
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
#ifdef ARDUINO_M5
  // initialize the M5StickC object
  auto cfg = M5.config();
  M5.begin(cfg);
  delay(500);
#else
  // initialize TFT screen
  tft.init(); // equivalent to tft.begin();
#ifdef TTGO
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
#endif
  tft.setBrightness(BRIGHTNESS);
#endif

  Serial.begin(115200);
  delay(500);  // needed by C6

  Serial.println("");
  Serial.println("CryptoCurrency candlestick chart display terminal started.");

#ifdef CDS
  pinMode(cds, INPUT);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  backlight_is_on = true;
#endif

  PHYSICAL_LCD.setRotation(DEFAULT_ROTATION + ROTATION_OFFSET); // set it to 1 or 3 for landscape resolution

  restoreSettings();

#ifdef USE_SPRITE
#ifdef E_INK
  canvas.setColorDepth(1);
#else
  canvas.setColorDepth(PHYSICAL_LCD.getColorDepth());
#endif

  // allow canvas to use PS RAM area.
  canvas.setPsram(true);
  
  canvas.createSprite(PHYSICAL_LCD.width(), PHYSICAL_LCD.height());
#endif

  tftWidth = LCD.width();
  tftHeight = LCD.height() / numScreens;
  tftHalfHeight = tftHeight / 2;
  LCD.fillScreen(COLOR_BG);

#ifdef E_INK
  PHYSICAL_LCD.setEpdMode(m5gfx::epd_mode_t::epd_quality);
#endif

  numSticks =
    (tftWidth < MAX_HORIZONTAL_RESOLUTION) ? tftWidth / STICK_WIDTH : NUM_STICKS;

  currencies[0].another = 1;

  LCD.setTextPadding(PADX); // seems no effect by this line.
  LCD.setTextSize(1);
  LCD.setFont(PRICEFONT); // Select a font for last price display
  PriceFontHeight = LCD.fontHeight(PRICEFONT) - PRICE_FONT_HEIGHT_ADJUSTMENT;

  unsigned priceHeight = PriceFontHeight + LCD.fontHeight(OTHER_CURRENCY_BASE_VALUE_FONT);
  if (MINIMUM_SEPARATABLE_HEIGHT < tftHeight - priceHeight) {
    dedicatedPriceAreaHeight = priceHeight;
    tftHeight -= priceHeight;
  }

  Serial.println("Attempting to connect to WiFi...");

  LCD.fillScreen(COLOR_CONNECTIONBG); // Clear screen for status display
  LCD.setTextColor(COLOR_TEXT);  
  LCD.drawString("Connecting ...",
		 PADX, LCD.height() / 2 - LCD.fontHeight(CONNECTINGFONT) / 2, CONNECTINGFONT);
  canvasPushSprite();

  for (int i = 0; i < wifi_count; i++) {
    wifiMulti.addAP(wifi_list[i].ssid, wifi_list[i].pass);
  }  
  wifiMulti.run();
  
#ifdef BUTTON1
  attachInterrupt(digitalPinToInterrupt(BUTTON1), buttonEventProc, FALLING);
#endif
#ifdef BUTTON2
  attachInterrupt(digitalPinToInterrupt(BUTTON2), buttonEventProc, FALLING);
#endif
  
  timer.setInterval(1000, SecProc);
  timer.setInterval(60000, _ShowCurrentPrice);
}
  
void loop()
{
#ifdef ARDUINO_M5
  M5.update();
  if (M5.BtnA.wasPressed()) {
    if (0 < alertDuration) {
      alertDuration = 0;
    }
    else {
      changeTriggered = true;
    }
  }
  else if (M5.BtnB.wasPressed()) {
    if (0 < alertDuration) {
      alertDuration = 0;
    }
    else {
      rotationTriggered = true;
    }
  }
  else if (M5.Touch.getCount() > 0) {
    auto detail = M5.Touch.getDetail(0); // Get information about the first finger.
    // The screen is only judged at the moment it is pressed.
    if (detail.wasPressed()) {
      if (0 < alertDuration) {
	alertDuration = 0;
      }
      else {
        bool touchUpperHalf = false;
#ifdef USE_SPRITE	
        int physicalHeight = M5.Display.height();
        int physicalWidth = M5.Display.width();
        unsigned r = LCD.getRotation() & 3;

        // Convert physical touch coordinates to logical top/bottom based on canvas rotation
        if (r == 0) {
          touchUpperHalf = (detail.y < physicalHeight / 2);
        } else if (r == 2) {
          touchUpperHalf = (detail.y > physicalHeight / 2);
        } else if (r == 1) {
          touchUpperHalf = (detail.x > physicalWidth / 2);
        } else if (r == 3) {
          touchUpperHalf = (detail.x < physicalWidth / 2);
        }
#else
	touchUpperHalf = (detail.y < LCD.height() / 2);
#endif
        if (touchUpperHalf) {
          changeTriggered = true;
        } else {
          rotationTriggered = true;
        }
      }
    }
  }
#elif defined(R28T)
  // for E32R28T
  static uint32_t lastTouchTime = 0;
  int32_t x, y;

  if (LCD.getTouch(&x, &y)) {
    if (millis() - lastTouchTime > 300) { 
      Serial.printf("Touch! X:%d, Y:%d\n", x, y);
      if (0 < alertDuration) {
        alertDuration = 0;
      } else {
	rotationTriggered = true;
      }
    }
    lastTouchTime = millis();
  }  
#endif
  timer.run();
}
