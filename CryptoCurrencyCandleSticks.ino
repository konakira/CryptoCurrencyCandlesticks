// #include <WiFi.h>
#include <WiFiClientSecure.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
//#include <Free_Fonts.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <SimpleTimer.h>
#include <TimeLib.h>

#include "auth.h"

/* root CA of bitbank.cc
MIIEDzCCAvegAwIBAgIBADANBgkqhkiG9w0BAQUFADBoMQswCQYDVQQGEwJVUzEl
MCMGA1UEChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjEyMDAGA1UECxMp
U3RhcmZpZWxkIENsYXNzIDIgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMDQw
NjI5MTczOTE2WhcNMzQwNjI5MTczOTE2WjBoMQswCQYDVQQGEwJVUzElMCMGA1UE
ChMcU3RhcmZpZWxkIFRlY2hub2xvZ2llcywgSW5jLjEyMDAGA1UECxMpU3RhcmZp
ZWxkIENsYXNzIDIgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwggEgMA0GCSqGSIb3
DQEBAQUAA4IBDQAwggEIAoIBAQC3Msj+6XGmBIWtDBFk385N78gDGIc/oav7PKaf
8MOh2tTYbitTkPskpD6E8J7oX+zlJ0T1KKY/e97gKvDIr1MvnsoFAZMej2YcOadN
+lq2cwQlZut3f+dZxkqZJRRU6ybH838Z1TBwj6+wRir/resp7defqgSHo9T5iaU0
X9tDkYI22WY8sbi5gv2cOj4QyDvvBmVmepsZGD3/cVE8MC5fvj13c7JdBmzDI1aa
K4UmkhynArPkPw2vCHmCuDY96pzTNbO8acr1zJ3o/WSNF4Azbl5KXZnJHoe0nRrA
1W4TNSNe35tfPe/W93bC6j67eA0cQmdrBNj41tpvi/JEoAGrAgEDo4HFMIHCMB0G
A1UdDgQWBBS/X7fRzt0fhvRbVazc1xDCDqmI5zCBkgYDVR0jBIGKMIGHgBS/X7fR
zt0fhvRbVazc1xDCDqmI56FspGowaDELMAkGA1UEBhMCVVMxJTAjBgNVBAoTHFN0
YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4xMjAwBgNVBAsTKVN0YXJmaWVsZCBD
bGFzcyAyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5ggEAMAwGA1UdEwQFMAMBAf8w
DQYJKoZIhvcNAQEFBQADggEBAAWdP4id0ckaVaGsafPzWdqbAYcaT1epoXkJKtv3
L7IezMdeatiDh6GX70k1PncGQVhiv45YuApnP+yz3SFmH8lU+nLMPUxA2IGvd56D
eruix/U0F47ZEUD0/CwqTRV/p2JdLiXTAAsgGh1o+Re49L2L7ShZ3U0WixeDyLJl
xy16paq8U4Zt3VekyvggQQto8PT7dL5WXXp59fkdheMtlb71cZBDzI0fmgAKhynp
VSJYACPq4xJDKVtHCN2MQWplBqjlIapBtJUhlbl90TSrE9atvNziPTnNvT51cKEY
WQPJIrSPnNVeKtelttQKbfi3QBFGmh95DmK/D5fs4C8fF5Q=
 */

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

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


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

#define CURRENCY_PAIR "eth_jpy"
#define CANDLE_STICK_FOOT_WIDTH "5min"
#define CANDLE_STICK_FOOT_WIDTH_NUM 5

#define PRICELINE 10000

#define STICK_WIDTH 3 // width of a candle stick
#define HORIZONTAL_RESOLUTION 240 // width of TTGO-T-display
#define NUM_STICKS (HORIZONTAL_RESOLUTION / STICK_WIDTH)
struct candlestick {
  unsigned startPrice, endPrice, lowestPrice, highestPrice;
  unsigned long timeStamp;
};

struct candlestick candlesticks[NUM_STICKS];

void
obtainSticks(unsigned n, unsigned long t)
{
  Serial.println("\nobtainSticks called.");

  while (0 < n) {
    if (!client.connect(SERVER, 443))
      Serial.println("\nConnection failed!");
    else {
      Serial.println("\nConnected to server!");

      // Make another HTTP request:
      client.print("GET https://" SERVER "/" CURRENCY_PAIR "/candlestick/" CANDLE_STICK_FOOT_WIDTH "/");
      {
	char yyyymmdd[9]; // 9 for "yyyymmdd"
	sprintf(yyyymmdd, "%04d%02d%02d", year(t), month(t), day(t));
	client.print(yyyymmdd);
	client.println(" HTTP/1.0");
      }
      client.println("Host: " SERVER);
      client.println("Connection: close");
      client.println();
    
      while (client.connected()) {
	String line = client.readStringUntil('\n');
	Serial.println(line); // echo response headers
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
	int success = doc["success"]; // 1

	JsonArray ohlcv = doc["data"]["candlestick"][0]["ohlcv"];
	unsigned numSticks = ohlcv.size();

	Serial.print("Success = ");
	Serial.println(success);

	Serial.print("Number of sticks = ");
	Serial.println(numSticks);
	if (n <= numSticks) { // enough sticks obtained
	  for (unsigned i = 0 ; i < n ; i++) {
	    // copy the last n data from JSON
	    unsigned ohlcvIndex = i + numSticks - n;
	    candlesticks[i].startPrice = ohlcv[ohlcvIndex][0].as<unsigned>();
	    candlesticks[i].highestPrice = ohlcv[ohlcvIndex][1].as<unsigned>();
	    candlesticks[i].lowestPrice = ohlcv[ohlcvIndex][2].as<unsigned>();
	    candlesticks[i].endPrice = ohlcv[ohlcvIndex][3].as<unsigned>();
	    candlesticks[i].timeStamp =
	      (unsigned long)(ohlcv[ohlcvIndex][5].as<unsigned long long>() / 1000);
	    unsigned long aho = (unsigned long)ohlcv[ohlcvIndex][5].as<unsigned long long>();
	  }
	  n = 0;
	}
	else {
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
}

#define MAX_SHORTER_PIXELVAL 134

unsigned prevPrice = 0;
#define PRICEBUFSIZE 24

void
ShowCurrentPrice()
{
  unsigned long t; // for current time
  char buf[PRICEBUFSIZE];
  unsigned lastPrice = 0;
  int lastPricePixel = 0;
  unsigned stickColor = TFT_RED, priceColor = TFT_GREEN;
  
  client.setCACert(bitbank_root_ca);

  Serial.println("\nStarting connection to server...");

  if (!client.connect(SERVER, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
    // Make a HTTP request:
    client.println("GET https://" SERVER "/" CURRENCY_PAIR "/ticker HTTP/1.0");
    client.println("Host: " SERVER);
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.println(line); // echo response headers
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
      Serial.println(F("Response:"));
      if (doc["success"]) {

	// Obtaining time stamp of ticker response and use it as the current time,
	// instead of obtaining current time by NTP.
	t = (unsigned long)(doc["data"]["timestamp"].as<unsigned long long>() / 1000);

	Serial.print("timestamp = ");
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
	Serial.println(minute(t));
	lastPrice = (unsigned)doc["data"]["last"].as<long>();

	itocsa(buf, PRICEBUFSIZE, lastPrice);
	Serial.print("last = ");
	Serial.println(buf);
	Serial.print("timestamp = ");
	Serial.println(t);
      }
    }
    client.stop();
  }
  obtainSticks(NUM_STICKS, t);

#define TIMEZONE (9 * 60 * 60)
  
  // show the sticks here
  unsigned lowest = candlesticks[0].lowestPrice;
  unsigned highest = candlesticks[0].highestPrice;
  unsigned prevHour = hour(candlesticks[0].timeStamp + TIMEZONE);
  for (unsigned i = 1 ; i < NUM_STICKS ; i++) {
    if (candlesticks[i].lowestPrice < lowest) {
      lowest = candlesticks[i].lowestPrice;
    }
    if (highest < candlesticks[i].highestPrice) {
      highest = candlesticks[i].highestPrice;
    }
  }
  Serial.print("lowest = ");
  Serial.println(lowest);
  Serial.print("highest = ");
  Serial.println(highest);

  Serial.print("hour = ");
  Serial.println(prevHour);

  Serial.print("timeStamp = ");
  Serial.println(candlesticks[0].timeStamp);

  Serial.print("place in pixel = ");
  Serial.println(map(candlesticks[NUM_STICKS - 1].endPrice, lowest, highest, MAX_SHORTER_PIXELVAL, 0));

  tft.fillScreen(TFT_BLACK);

#define TFT_DARKBLUE        0x000F      /*   0,   0, 127 */

  for (unsigned i = lowest / PRICELINE + 1 ; i * PRICELINE < highest ; i++) {
    unsigned y = map(i * PRICELINE, lowest, highest, MAX_SHORTER_PIXELVAL, 0);
    tft.drawFastHLine(0, y, HORIZONTAL_RESOLUTION, TFT_DARKBLUE);
  }
  
  for (unsigned i = 0 ; i < NUM_STICKS ; i++) {
    int lowestPixel, highestPixel, lowPixel, pixelHeight;
    
    lowestPixel = map(candlesticks[i].lowestPrice, lowest, highest, MAX_SHORTER_PIXELVAL, 0);
    highestPixel = map(candlesticks[i].highestPrice, lowest, highest, MAX_SHORTER_PIXELVAL, 0);

    if (candlesticks[i].startPrice < candlesticks[i].endPrice) {
      lowPixel = map(candlesticks[i].endPrice, lowest, highest, MAX_SHORTER_PIXELVAL, 0);
      pixelHeight = map(candlesticks[i].startPrice, lowest, highest, MAX_SHORTER_PIXELVAL, 0)
	- lowPixel;
      stickColor = TFT_GREEN;
    }
    else {
      lowPixel = map(candlesticks[i].startPrice, lowest, highest, MAX_SHORTER_PIXELVAL, 0);
      pixelHeight = map(candlesticks[i].endPrice, lowest, highest, MAX_SHORTER_PIXELVAL, 0)
	- lowPixel;
      stickColor = TFT_RED;
    }

    lastPricePixel = map(lastPrice, lowest, highest, MAX_SHORTER_PIXELVAL, 0);
    if (lastPrice < prevPrice) {
      priceColor = TFT_RED;
    }
    prevPrice = lastPrice;

    unsigned curHour = hour(candlesticks[i].timeStamp + TIMEZONE);
    if (curHour != prevHour) {
      prevHour = curHour;
      tft.drawFastVLine(i * 3 + 1, 0, MAX_SHORTER_PIXELVAL, TFT_DARKBLUE);
      if (curHour % 3 == 0) {
	if (i * 3 - 5 < HORIZONTAL_RESOLUTION - tft.textWidth(buf, 2) - 10) {
	  // if we have enough space around vertical line, draw the time
	  unsigned textY = 0;

	  if (lastPricePixel < MAX_SHORTER_PIXELVAL / 2) {
	    textY = MAX_SHORTER_PIXELVAL - tft.fontHeight(2);
	  }
	  tft.setTextColor(TFT_WHITE);
	  tft.drawNumber(curHour, i * 3 - 5, textY, 2);
	}
      }
    }

    tft.drawFastVLine(i * 3 + 1, highestPixel, lowestPixel - highestPixel, TFT_LIGHTGREY);
    tft.fillRect(i * 3, highestPixel, 3, pixelHeight, stickColor);
  }

#define PRICE_MIN_X 5
#define PRICE_PAD_X 3
#define PRICE_PAD_Y 10
#define BORDER_WIDTH 2

  // draw highest and lowest price in the chart
  itocsa(buf, PRICEBUFSIZE, highest);
  tft.setTextColor(TFT_BLACK);
  tft.drawString(buf, HORIZONTAL_RESOLUTION - tft.textWidth(buf, 2) - 2, -1, 2);
  tft.drawString(buf, HORIZONTAL_RESOLUTION - tft.textWidth(buf, 2), 1, 2);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(buf, HORIZONTAL_RESOLUTION - tft.textWidth(buf, 2) - 1, 0, 2);

  itocsa(buf, PRICEBUFSIZE, lowest);
  tft.setTextColor(TFT_BLACK);
  tft.drawString(buf,
		 HORIZONTAL_RESOLUTION - tft.textWidth(buf, 2) - 2,
		 MAX_SHORTER_PIXELVAL - tft.fontHeight(2) - 1, 2);
  tft.drawString(buf,
		 HORIZONTAL_RESOLUTION - tft.textWidth(buf, 2),
		 MAX_SHORTER_PIXELVAL - tft.fontHeight(2) + 1, 2);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(buf,
		 HORIZONTAL_RESOLUTION - tft.textWidth(buf, 2) - 1,
		 MAX_SHORTER_PIXELVAL - tft.fontHeight(2), 2);

  // draw last price
  itocsa(buf, PRICEBUFSIZE, lastPrice);
  unsigned stringWidth = tft.textWidth(buf, 6) + PRICE_PAD_X;
  
  // show the current ETH price on TTGO-T-display
  // The following is a quite tentative code. To be updated.
  tft.drawFastHLine(0, lastPricePixel, PRICE_MIN_X, priceColor);
  tft.drawFastHLine(stringWidth, lastPricePixel, HORIZONTAL_RESOLUTION - stringWidth, priceColor);
  // tft.drawFastHLine(0, lastPricePixel, HORIZONTAL_RESOLUTION, priceColor);

  int textY = lastPricePixel - (tft.fontHeight(6) / 2);
  if (textY < 0) {
    textY = 0;
  }
  else if (MAX_SHORTER_PIXELVAL - tft.fontHeight(6) + PRICE_PAD_Y < textY) {
    textY = MAX_SHORTER_PIXELVAL - tft.fontHeight(6) + PRICE_PAD_Y;
  }
  //  tft.setFreeFont(FF20);
  //  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK);
  tft.drawString(buf, - BORDER_WIDTH, textY - BORDER_WIDTH, 6);
  tft.drawString(buf, BORDER_WIDTH, textY + BORDER_WIDTH, 6);
  tft.setTextColor(priceColor);
  tft.drawString(buf, 0, textY, 6);
  //  tft.setTextSize(1);
  //  tft.drawString(buf, 0, textY, GFXFF);
}

void setup()
{
  Serial.begin(115200);
  
  Serial.println("");

  // initialize TFT screen
  tft.init(); // equivalent to tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Connecting ...",
		 0, MAX_SHORTER_PIXELVAL / 2 - tft.fontHeight(4) / 2, 4);
  tft.setTextSize(1);

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

  ShowCurrentPrice();

  Serial.println("");
  
  timer.setInterval(60000, ShowCurrentPrice);
}

void loop()
{
  timer.run();
}
