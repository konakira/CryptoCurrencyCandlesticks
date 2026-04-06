# CryptoCurrencyCandlesticks

![Running Image](images/M5family.jpeg "Running Image")

A real-time cryptocurrency candlestick chart display terminal for ESP32 and M5Stack series devices. It fetches data from the bitbank.cc API to render live market movements directly on your desktop.

The specifications of the API can be found here: https://github.com/bitbankinc/bitbank-api-docs
This program uses the public candlestick API documented here: https://github.com/bitbankinc/bitbank-api-docs/blob/master/public-api.md
**No API key is required** to access this API.

## Key Features

* **Dynamic Dual Display:** Automatically adapts to your device's screen resolution. Enjoy a split-screen view showing both BTC and ETH simultaneously on larger screens (like M5Stack Core2), or toggle between them seamlessly on compact devices.
* **Smart Visual Alerts:** Never miss a critical market movement. The entire (or a large part of) the screen actively flashes (Green for surges, Red for drops) when breaking today's high/low, or when detecting a sudden price change (e.g., >1% movement) within a 1-minute or 5-minute window.
* **Multi-Currency Tracking:** Tracks BTC/JPY and ETH/JPY pairs by default, rendering 5-minute candlestick charts with real-time ticker updates.
* **Relative Strength Tracker:** Features a cleverly overlaid orange line graph that visualizes the real-time relative value (ratio) between the two tracked cryptocurrencies.
* **Universal Compatibility:** A single, unified codebase powered by PlatformIO. It supports a wide range of devices including M5StickC/Plus, M5Stack Core2, M5StickS3 (M5Unified), and custom ESP32/ESP32-C6 setups.

## Setup & Configuration

1. Rename `auth.h.example` to `auth.h` and enter your Wi-Fi credentials.
2. Select your target environment in PlatformIO (e.g., `env:m5stick-c-plus`, `env:m5stack-core2`, `env:m5stick_s3`).
3. Build and upload!

## Acknowledgements

* `Free_Fonts.h` is included in this repository for convenience. It was originally created by [Bodmer](https://github.com/Bodmer) for the [TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI/blob/master/examples/320%20x%20240/Free_Font_Demo/Free_Fonts.h), which elegantly wraps the free fonts provided by the [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library). Huge thanks to the original authors!