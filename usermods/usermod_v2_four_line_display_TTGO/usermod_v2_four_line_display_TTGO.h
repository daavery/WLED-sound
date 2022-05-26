// Credits to @mrVanboy, @gwaland and my dearest friend @westward
// Also for @spiff72 for usermod TTGO-T-Display
// 210217
#pragma once

#include "wled.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WIRE.h>
#ifdef ARDUINO_ARCH_ESP32
  #define HW_PIN_SCL 22
  #define HW_PIN_SDA 21
  #define HW_PIN_CLOCKSPI 18
  #define HW_PIN_DATASPI 23
  #ifndef FLD_PIN_SCL
    #define FLD_PIN_SCL 22
  #endif
  #ifndef FLD_PIN_SDA
    #define FLD_PIN_SDA 21
  #endif
  #ifndef FLD_PIN_CLOCKSPI
    #define FLD_PIN_CLOCKSPI 18
  #endif
   #ifndef FLD_PIN_DATASPI
    #define FLD_PIN_DATASPI 23
  #endif
  #ifndef FLD_PIN_DC
    #define FLD_PIN_DC 19
  #endif
  #ifndef FLD_PIN_CS
    #define FLD_PIN_CS 5
  #endif
  #ifndef FLD_PIN_RESET
    #define FLD_PIN_RESET 26
  #endif
#else
  #define HW_PIN_SCL 5
  #define HW_PIN_SDA 4
  #define HW_PIN_CLOCKSPI 14
  #define HW_PIN_DATASPI 13
  #ifndef FLD_PIN_SCL
    #define FLD_PIN_SCL 5
  #endif
  #ifndef FLD_PIN_SDA
    #define FLD_PIN_SDA 4
  #endif
  #ifndef FLD_PIN_CLOCKSPI
    #define FLD_PIN_CLOCKSPI 14
  #endif
   #ifndef FLD_PIN_DATASPI
    #define FLD_PIN_DATASPI 13
  #endif
  #ifndef FLD_PIN_DC
    #define FLD_PIN_DC 12
  #endif
    #ifndef FLD_PIN_CS
    #define FLD_PIN_CS 15
  #endif
  #ifndef FLD_PIN_RESET
    #define FLD_PIN_RESET 16
  #endif
#endif

//#define USERMOD_ID_ST7789_DISPLAY 97

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23

#define TFT_BL          4  // Display backlight control pin

#define SCREEN_TIMEOUT_MS  60*1000    // 1 min

#define TIME_INDENT        0
#define DATE_INDENT        2

#define FLD_TYPE ST7789
// Minimum time between redrawing screen in ms
#define USER_LOOP_REFRESH_RATE_MS 1000

// Extra char (+1) for null
#define LINE_BUFFER_SIZE         16+1
typedef enum {
  FLD_LINE_BRIGHTNESS = 0,
  FLD_LINE_EFFECT_SPEED,
  FLD_LINE_EFFECT_INTENSITY,
  FLD_LINE_EFFECT_CUSTOM1, //WLEDSR
  FLD_LINE_EFFECT_CUSTOM2, //WLEDSR
  FLD_LINE_EFFECT_CUSTOM3, //WLEDSR
  FLD_LINE_MODE,
  FLD_LINE_PALETTE,
  FLD_LINE_PRESET, //WLEDSR
  FLD_LINE_TIME,
  FLD_LINE_SQUELCH,
  FLD_LINE_GAIN,
  FLD_LINE_OTHER, //WLEDSR: no Line4Type
  FLD_LINE_NULL //WLEDSR: no Line4Type
} Line4Type;

typedef enum {
  NONE = 0,
  SSD1306,      // U8X8_SSD1306_128X32_UNIVISION_HW_I2C
  SH1106,       // U8X8_SH1106_128X64_WINSTAR_HW_I2C
  SSD1306_64,   // U8X8_SSD1306_128X64_NONAME_HW_I2C
  SSD1305,      // U8X8_SSD1305_128X32_ADAFRUIT_HW_I2C
  SSD1305_64,   // U8X8_SSD1305_128X64_ADAFRUIT_HW_I2C
  SSD1306_SPI,  // U8X8_SSD1306_128X32_NONAME_HW_SPI
  SSD1306_SPI64,// U8X8_SSD1306_128X64_NONAME_HW_SPI
  SH1106_SPI,    // U8X8_SH1106_128X64_WINSTAR_HW_SPI
  ST7789
} DisplayType;

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

// How often we are redrawing screen
#define USER_LOOP_REFRESH_RATE_MS 1000

    char sliderNames[5][LINE_BUFFER_SIZE*2]; //WLEDSR
    const char sliderDefaults[5][LINE_BUFFER_SIZE] = {"FX Speed", "FX Intens.", "FX Custom1", "FX Custom2", "FX Custom3"}; //WLEDSR


//class name. Use something descriptive and leave the ": public Usermod" part :)
class FourLineDisplayUsermod : public Usermod {
  private:
    //Private class members. You can declare variables and functions only accessible to your usermod here
    bool initDone = false;
    bool enabled = true;
    unsigned long lastTime = 0;

    // needRedraw marks if redraw is required to prevent often redrawing.
    bool needRedraw = true;
    uint8_t tftcharwidth = 19;  // Number of chars that fit on screen with text size set to 2
	  #ifndef FLD_SPI_DEFAULT
    int8_t ioPin[5] = {FLD_PIN_SCL, FLD_PIN_SDA, -1, -1, -1};        // I2C pins: SCL, SDA
    uint32_t ioFrequency = 400000;  // in Hz (minimum is 100000, baseline is 400000 and maximum should be 3400000)
    #else
    int8_t ioPin[5] = {FLD_PIN_CLOCKSPI, FLD_PIN_DATASPI, FLD_PIN_CS, FLD_PIN_DC, FLD_PIN_RESET}; // SPI pins: CLK, MOSI, CS, DC, RST
    uint32_t ioFrequency = 1000000;  // in Hz (minimum is 500kHz, baseline is 1MHz and maximum should be 20MHz)
    #endif
    DisplayType type = FLD_TYPE;    // display type

    bool flip = false;              // flip display 180°
    uint8_t contrast = 10;          // screen contrast
    uint8_t lineHeight = 1;         // 1 row or 2 rows
    uint32_t refreshRate = USER_LOOP_REFRESH_RATE_MS; // in ms
    uint32_t screenTimeout = SCREEN_TIMEOUT_MS;       // in ms

    bool sleepMode = true;          // allow screen sleep?
    bool clockMode = false;         // display clock
    bool forceAutoRedraw = false;         // WLEDSR: force rotating of variables in display, even if strip.isUpdating, this can cause led stutter, this should not be necessary if display is fast enough...
    bool noAutoRedraw = false;         // WLEDSR: never do auto Redraw, only when variable changes or rotary is pressed (in case redraw causes stutter on leds, should not be needed with spi displays)

    // Next variables hold the previous known values to determine if redraw is
    // required.
    String knownSsid = "";
    IPAddress knownIp;
    uint8_t knownBrightness = 0;
    uint8_t knownEffectSpeed = 0;
    uint8_t knownEffectIntensity = 0;
    uint8_t knownEffectCustom1 = 0; //WLEDSR
    uint8_t knownEffectCustom2 = 0; //WLEDSR
    uint8_t knownEffectCustom3 = 0; //WLEDSR
    uint8_t knownMode = 0;
    uint8_t knownPalette = 0;
    uint8_t knownMinute = 99;
    uint8_t knownHour = 99;
    uint8_t knownSquelch = 99;
    uint8_t knownGain = 99;

    uint8_t knownClockMode = 0;

    bool displayTurnedOff = false;
    unsigned long lastUpdate = 0;
    unsigned long lastRedraw = 0;
    unsigned long overlayUntil = 0;
    Line4Type lineType = FLD_LINE_BRIGHTNESS;
    // Set to 2 or 3 to mark lines 2 or 3. Other values ignored.
    byte markLineNum = 0;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    static const char _contrast[];
    static const char _refreshRate[];
    static const char _screenTimeOut[];
    static const char _flip[];
    static const char _sleepMode[];
    static const char _clockMode[];
    static const char _forceAutoRedraw[]; //WLEDSR
    static const char _noAutoRedraw[]; //WLEDSR
    static const char _busClkFrequency[];


  public:
    //Functions called by WLED

    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup()
    {
        tft.init();
        tft.setRotation(3);  //Rotation here is set up for the text to be readable with the port on the left. Use 1 to flip.
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED);
        tft.setCursor(60, 100);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(2);
        tft.print("Loading...");
        if (TFT_BL > 0) 
        { // TFT_BL has been set in the TFT_eSPI library
         pinMode(TFT_BL, OUTPUT); // Set backlight pin to output mode
         digitalWrite(TFT_BL, HIGH); // Turn backlight on.
        }
		
      initDone = true;
    }

    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {
      //Serial.println("Connected to WiFi!");
	   needRedraw = true;
    }

    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     *
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     *
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop() {
// Check if we time interval for redrawing passes.
//    if (millis() - lastUpdate < USER_LOOP_REFRESH_RATE_MS)
//        {
//            return;
//        }
   if (!enabled || millis() - lastUpdate < (clockMode?1000:refreshRate) || strip.isUpdating()) return;
      lastUpdate = millis();

        // WWLEDSR: redraw if
      //      -- timer and (forcedAutoUpdate or strip idle) and autoRedraw
      //   OR
      //      -- any variable updated (including clockmode, hours and minutes)
      //   OR
      //      -- sleepmode and screentimeout
      // Note wakeDispay (used by rotaty) triggers its own redraw
      if ( ((forceAutoRedraw || !strip.isUpdating()) && !noAutoRedraw) || checkChangedType() != FLD_LINE_NULL || (sleepMode && (millis() - lastRedraw > screenTimeout)))
        redraw(false);
    }
     /**
     * Wrappers for screen drawing
     */
    void setFlipMode(uint8_t mode) {
      if (type == NONE || !enabled) return;
  // DAA //    u8x8->setFlipMode(mode);
    }
    void setContrast(uint8_t contrast) {
      if (type == NONE || !enabled) return;
    // DAA //     u8x8->setContrast(contrast);
    }
    void drawString(uint8_t col, uint8_t row, const char *string, bool ignoreLH=false) {
      if (type == NONE || !enabled) return;
    // DAA //     u8x8->setFont(u8x8_font_chroma48medium8_r);
      if (!ignoreLH && lineHeight==2)   // DAA // u8x8->draw1x2String(col, row, string)
          {
          tft.setTextSize(3);
          tft.setTextPadding(200);
          tft.setCursor(col*16, row*25);
          tft.print(string);
        } 
      else                              // DAA // u8x8->drawString(col, row, string)
        {
          tft.setTextSize(2);
          tft.setTextPadding(200);
          tft.setCursor(col*16, row*25);
          tft.print(string);
        } 
    }
    void draw2x2String(uint8_t col, uint8_t row, const char *string) {
      if (type == NONE || !enabled) return;
    // DAA //     u8x8->setFont(u8x8_font_chroma48medium8_r);
     // DAA //    u8x8->draw2x2String(col, row, string);
          tft.setTextSize(3);
          tft.setTextPadding(200);
          tft.setCursor(col*16, row*25);
          tft.print(string);
        } 

    void drawGlyph(uint8_t col, uint8_t row, char glyph,  uint8_t font, bool ignoreLH=false) {
      if (type == NONE || !enabled) return;
     // DAA //    u8x8->setFont(font);
	     tft.setTextFont(  font);
      if (!ignoreLH && lineHeight==2)   // DAA //  u8x8->draw1x2Glyph(col, row, glyph)
         tft.drawChar(col,row,glyph);
      else 
         tft.drawChar(col,row,glyph);		  // DAA // u8x8->drawGlyph(col, row, glyph)
      ;
    }
    uint8_t getCols() {
      if (type==NONE || !enabled) return 0;
        return  16; // DAA // u8x8->getCols();
    }
    void clear() {
      if (type == NONE || !enabled) return;
          tft.fillScreen(TFT_BLACK);

     // DAA //    u8x8->clear();
    }
    void setPowerSave(uint8_t save) {
      if (type == NONE || !enabled) return;
        if (save)
               digitalWrite(TFT_BL, LOW); // Turn backlight on.
        else
               digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on.

 
    }




	    Line4Type checkChangedType() {
      if (((apActive) ? String(apSSID) : WiFi.SSID()) != knownSsid)
        return FLD_LINE_OTHER;
      else if (knownIp != (apActive ? IPAddress(4, 3, 2, 1) : Network.localIP()))
        return FLD_LINE_OTHER;
      else if (knownBrightness != bri)
        return FLD_LINE_BRIGHTNESS;
      else if (knownEffectSpeed != effectSpeed)
        return FLD_LINE_EFFECT_SPEED;
      else if (knownEffectIntensity != effectIntensity)
        return FLD_LINE_EFFECT_INTENSITY;
      else if (knownEffectCustom1 != effectCustom1)
        return FLD_LINE_EFFECT_CUSTOM1;
      else if (knownEffectCustom2 != effectCustom2)
        return FLD_LINE_EFFECT_CUSTOM2;
      else if (knownEffectCustom3 != effectCustom3)
        return FLD_LINE_EFFECT_CUSTOM3;
      else if (knownMode != strip.getMainSegment().mode)
        return FLD_LINE_MODE;
      else if (knownPalette != strip.getMainSegment().palette)
        return FLD_LINE_PALETTE;
      else if (knownSquelch != soundSquelch)
        return FLD_LINE_SQUELCH;
      else if (knownGain != sampleGain)
        return FLD_LINE_GAIN;
      else if (knownClockMode != clockMode || (clockMode && (knownHour != hour(localTime) || knownMinute != minute(localTime))))
        return FLD_LINE_OTHER;
      else
        return FLD_LINE_NULL;
    }

	    void center(String &line, uint8_t width) {
      bool tooLong = line.length()>width;
      line = line.substring(0, width);
      int len = line.length();
      if (len<width) for (byte i=(width-len)/2; i>0; i--) line = ' ' + line;
      for (byte i=line.length(); i<width; i++) line += ' ';
      // Print `~` char to indicate that line is longer, than our display
      if (tooLong)
         line[len-1] = '~';
    }
	
	    void redraw(bool forceRedraw) {
      static bool showName = false;
      unsigned long now = millis();

      if (!enabled) return;
      if (overlayUntil > 0) {
        if (now >= overlayUntil) {
          // Time to display the overlay has elapsed.
          overlayUntil = 0;
          forceRedraw = true;
        } else {
          // We are still displaying the overlay
          // Don't redraw.
          return;
        }
      }

      // Check if values which are shown on display changed from the last time.
      if (forceRedraw ||
          (((apActive) ? String(apSSID) : WiFi.SSID()) != knownSsid) ||
          (knownIp != (apActive ? IPAddress(4, 3, 2, 1) : Network.localIP())) ||
          (knownBrightness != bri) ||
          (knownEffectSpeed != effectSpeed) ||
          (knownEffectIntensity != effectIntensity) ||
          (knownMode != strip.getMainSegment().mode) ||
          (knownPalette != strip.getMainSegment().palette)) {
        knownHour = 99;   // force time update
        lastRedraw = now; // update lastRedraw marker
      } else if (sleepMode && !displayTurnedOff && ((now - lastRedraw)/1000)%5 == 0) {
        // change line every 5s
        showName = !showName;
        switch (lineType) {
          case FLD_LINE_BRIGHTNESS:
            lineType = FLD_LINE_EFFECT_SPEED;
            break;
          case FLD_LINE_MODE:
            lineType = FLD_LINE_BRIGHTNESS;
            break;
          case FLD_LINE_PALETTE:
            lineType = clockMode ? FLD_LINE_MODE : FLD_LINE_BRIGHTNESS;
            break;
          case FLD_LINE_EFFECT_SPEED:
            lineType = FLD_LINE_EFFECT_INTENSITY;
            break;
          case FLD_LINE_EFFECT_INTENSITY:
            lineType = FLD_LINE_EFFECT_CUSTOM1; //WLEDSR
            break;
          case FLD_LINE_EFFECT_CUSTOM1:
            lineType = FLD_LINE_EFFECT_CUSTOM2; //WLEDSR
            break;
          case FLD_LINE_EFFECT_CUSTOM2:
            lineType = FLD_LINE_EFFECT_CUSTOM3; //WLEDSR
            break;
          case FLD_LINE_EFFECT_CUSTOM3:
            lineType = FLD_LINE_PRESET;
            break;
          case FLD_LINE_PRESET:
            lineType = FLD_LINE_SQUELCH;
            break;
          case FLD_LINE_SQUELCH:
            lineType = FLD_LINE_GAIN;
            break;
          case FLD_LINE_GAIN:
            lineType = FLD_LINE_PALETTE;
            break;
          default:
            lineType = FLD_LINE_MODE;
            break;
        }

        //WLEDSR: Skip slider if not used
        if (lineType == FLD_LINE_EFFECT_SPEED && strlen_P(sliderNames[0]) == 0) //slidername empty
          lineType = FLD_LINE_EFFECT_INTENSITY;
        if (lineType == FLD_LINE_EFFECT_INTENSITY && strlen_P(sliderNames[1]) == 0)
          lineType = FLD_LINE_EFFECT_CUSTOM1;
        if (lineType == FLD_LINE_EFFECT_CUSTOM1 && strlen_P(sliderNames[2]) == 0)
          lineType = FLD_LINE_EFFECT_CUSTOM2;
        if (lineType == FLD_LINE_EFFECT_CUSTOM2 && strlen_P(sliderNames[3]) == 0)
          lineType = FLD_LINE_EFFECT_CUSTOM3;
        if (lineType == FLD_LINE_EFFECT_CUSTOM3 && strlen_P(sliderNames[4]) == 0)
          lineType = FLD_LINE_PRESET;
        if (lineType == FLD_LINE_PRESET && currentPreset == -1)
          lineType = FLD_LINE_SQUELCH;

        knownHour = 99; // force time update
      } // do not update lastRedraw marker if just switching row content
      else {
        // Nothing to change.
        // Turn off display after 3 minutes with no change.
        if (sleepMode && !displayTurnedOff && (millis() - lastRedraw > screenTimeout)) {
          // We will still check if there is a change in redraw()
          // and turn it back on if it changed.
          sleepOrClock(true);
        }
        else if (displayTurnedOff && clockMode) {
          showTime();
        }
        return;
      }

      // Turn the display back on
      if (displayTurnedOff) sleepOrClock(false);

      // Update last known values.
      knownSsid = apActive ? WiFi.softAPSSID() : WiFi.SSID();
      knownIp = apActive ? IPAddress(4, 3, 2, 1) : Network.localIP();
      knownBrightness = bri;
      knownMode = strip.getMainSegment().mode;
      knownPalette = strip.getMainSegment().palette;
      knownEffectSpeed = effectSpeed;
      knownEffectIntensity = effectIntensity;
      knownEffectCustom1 = effectCustom1; //WLEDSR
      knownEffectCustom2 = effectCustom2; //WLEDSR
      knownEffectCustom3 = effectCustom3; //WLEDSR
      knownSquelch = soundSquelch;
      knownGain = sampleGain;

      knownClockMode = clockMode;

      // Do the actual drawing
	  clear();
      String line;
      // First row with Wifi name
      // DAA //drawGlyph(0, 0, 80, u8x8_font_open_iconic_embedded_1x1); // home icon
      line = knownSsid;
      center(line, getCols()-1);
      drawString(1, 0, line.c_str());
      // Print `~` char to indicate that SSID is longer, than our display
      if (knownSsid.length() > (int)getCols()-1) {
        drawString(getCols() - 1, 0, "~");
      }

      // Second row with IP or Psssword
      // DAA //drawGlyph(0, lineHeight, 68, u8x8_font_open_iconic_embedded_1x1); // wifi icon
      // Print password in AP mode and if led is OFF.
      if (apActive && bri == 0) {
        drawString(1, lineHeight, apPass);
      } else {
        // alternate IP address and server name
        line = knownIp.toString();
        if (showName && strcmp(serverDescription, "WLED") != 0) {
          line = serverDescription;
        }
        center(line, getCols()-1);
        drawString(1, lineHeight, line.c_str());
      }

      // draw third and fourth row
      drawLine(3, clockMode ? lineType : FLD_LINE_MODE);
      drawLine(4, clockMode ? FLD_LINE_TIME : lineType);

      //WLEDSR: Show icon per variable
      if (clockMode)
        drawGlyph(0, 3*lineHeight, 65,2);//, u8x8_font_open_iconic_embedded_1x1); // clock icon
      //else done in showCurrentEffectOrPalette

      switch (lineType) {
        case FLD_LINE_BRIGHTNESS:
        drawGlyph(0, (clockMode?2:3)*lineHeight, 66 + (bri > 0 ? 3 : 0), 2); // sun/moon icon
          break;
        case FLD_LINE_MODE: //nothing displayed as this is done in showCurrentEffectOrPalette
          break;
        case FLD_LINE_PALETTE: //nothing displayed as this is done in showCurrentEffectOrPalette
          break;
        case FLD_LINE_EFFECT_SPEED:
        // DAA //  drawGlyph(0, (clockMode?2:3)*lineHeight, 72, u8x8_font_open_iconic_play_1x1); // fast forward icon
          break;
        case FLD_LINE_EFFECT_INTENSITY:
        // DAA //  drawGlyph(0, (clockMode?2:3)*lineHeight, 78, u8x8_font_open_iconic_thing_1x1); // kind of fire icon
          break;
        case FLD_LINE_EFFECT_CUSTOM1:
         // DAA // drawGlyph(0, (clockMode?2:3)*lineHeight, 68, u8x8_font_open_iconic_weather_1x1); // star icon
          break;
        case FLD_LINE_EFFECT_CUSTOM2:
        // DAA //  drawGlyph(0, (clockMode?2:3)*lineHeight, 68, u8x8_font_open_iconic_weather_1x1); // star icon
          break;
        case FLD_LINE_EFFECT_CUSTOM3:
        // DAA //  drawGlyph(0, (clockMode?2:3)*lineHeight, 68, u8x8_font_open_iconic_weather_1x1); // star icon
          break;
        case FLD_LINE_PRESET:
        // DAA //  drawGlyph(0, (clockMode?2:3)*lineHeight, 88, u8x8_font_open_iconic_arrow_1x1); // circle like icon
          break;
        case FLD_LINE_TIME:
        // DAA //  drawGlyph(0, (clockMode?2:3)*lineHeight, 65, u8x8_font_open_iconic_embedded_1x1); //  clock icon
          break;
        case FLD_LINE_SQUELCH:
        // DAA //  drawGlyph(0, (clockMode?2:3)*lineHeight, 65, u8x8_font_open_iconic_embedded_1x1); //  clock icon
          break;
        case FLD_LINE_GAIN:
        // DAA //  drawGlyph(0, (clockMode?2:3)*lineHeight, 65, u8x8_font_open_iconic_embedded_1x1); //  clock icon
          break;
        default:
           drawGlyph(0, (clockMode?2:3)*lineHeight, 72, 2); //  icon
          break;
      }
      if (markLineNum>1) drawGlyph(2, markLineNum*lineHeight, 66, 2); // arrow icon
    }

    //WLEDSR: Use custom slidernames
    void drawLine(uint8_t line, Line4Type lineType) {
      char lineBuffer[LINE_BUFFER_SIZE];
      uint8_t printedChars;
      switch(lineType) {
        case FLD_LINE_BRIGHTNESS:
          sprintf_P(lineBuffer, PSTR("Brightness  %3d"), bri);
          drawString(1, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_EFFECT_SPEED:
          sprintf_P(lineBuffer, PSTR("%.11s %3d"), sliderNames[0], effectSpeed);
          drawString(1, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_EFFECT_INTENSITY:
          sprintf_P(lineBuffer, PSTR("%.11s %3d"), sliderNames[1], effectIntensity);
          drawString(1, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_EFFECT_CUSTOM1: //WLEDSR
          sprintf_P(lineBuffer, PSTR("%.11s %3d"), sliderNames[2], effectCustom1);
          drawString(1, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_EFFECT_CUSTOM2: //WLEDSR
          sprintf_P(lineBuffer, PSTR("%.11s %3d"), sliderNames[3], effectCustom2);
          drawString(1, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_EFFECT_CUSTOM3: //WLEDSR
          sprintf_P(lineBuffer, PSTR("%.11s %3d"), sliderNames[4], effectCustom3);
          drawString(1, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_PRESET:
          sprintf_P(lineBuffer, PSTR("FX Preset   %3d"), currentPreset);
          drawString(1, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_MODE:
          printedChars = extractModeName(knownMode, JSON_mode_names, lineBuffer, LINE_BUFFER_SIZE-1);
          for (;printedChars < getCols()-2 && printedChars < LINE_BUFFER_SIZE-3; printedChars++) lineBuffer[printedChars]=' ';
          lineBuffer[printedChars] = 0;
          drawString(2, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_PALETTE:
          printedChars = extractModeName(knownPalette, JSON_palette_names, lineBuffer, LINE_BUFFER_SIZE-1);
          for (;printedChars < getCols()-2 && printedChars < LINE_BUFFER_SIZE-3; printedChars++) lineBuffer[printedChars]=' ';
          lineBuffer[printedChars] = 0;
          drawString(2, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_SQUELCH:
          sprintf_P(lineBuffer, PSTR("Squelch     %3d"), soundSquelch);
          drawString(1, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_GAIN:
          sprintf_P(lineBuffer, PSTR("Gain        %3d"), sampleGain);
          drawString(1, line*lineHeight, lineBuffer);
          break;
        case FLD_LINE_TIME:
        default:
          showTime(false);
          break;
      }
    }

    /**
     * Display the current effect or palette (desiredEntry)
     * on the appropriate line (row).
     */
    void showCurrentEffectOrPalette(int knownMode, const char *qstring, uint8_t row) {
      char lineBuffer[LINE_BUFFER_SIZE];
      uint8_t printedChars = 0;
      char singleJsonSymbol;

      //WLEDSR
      bool insideQuotes = false;
      uint8_t qComma = 0;
      bool insideSliders = false;
      uint8_t printedCharsSliders = 0;
      uint8_t sliderCounter = 0;
      bool noteFound = false;
      bool atSignFound = false;
      bool isEffect = strlen_P(qstring) > 2000; //currently 777 vs 4451

      // Find the mode name in JSON
      //WLEDSR: check for dynamic sliders
      DEBUG_PRINTLN("Display new effect started");
      for (size_t i = 0; i < strlen_P(qstring); i++) {
        singleJsonSymbol = pgm_read_byte_near(qstring + i);
        if ((singleJsonSymbol == ']') && (qComma == knownMode)) break;       //WLEDSR(HarryB): Stop reading, we've got all we want
        if (singleJsonSymbol == '\0') break;
        switch (singleJsonSymbol) {
          case '"':
            if (!insideQuotes) { //begin of string
              noteFound = false;
              sliderCounter = 0; //start counting sliders
              atSignFound = false;
            }
            insideQuotes = !insideQuotes;
            break;
          case '[':
            break;
          case 226: //WLEDSR: first byte of the 3 byte note character is for ♪ and ♫ 226
            noteFound = true;
            break;
          case '@':
            printedCharsSliders = 0; //set char counter to 0
            insideSliders = true;
            atSignFound = true;
            break;
          case '!':
            if (insideSliders) {
              sprintf_P(sliderNames[sliderCounter], sliderDefaults[sliderCounter]);
              printedCharsSliders = strlen_P(sliderNames[sliderCounter]); //set char counter to 0
            }
            break;
          case ',':
            if (!insideQuotes)       //WLEDSR(HarryB) added condition to differentiate between comma in mode name or as seperator
              qComma++;
            if (insideSliders) { //WLEDSR: if still in slider parsing, comma is end of string
              sliderNames[sliderCounter][printedCharsSliders] = 0; //end of string
              sliderCounter++; //go to next slider
              printedCharsSliders = 0; //set char counter to 0
            }
            break;
          case ';':
            if (insideSliders) { //if still parsing sliders
              sliderNames[sliderCounter][printedCharsSliders] = 0; //end of string
              sliderCounter++; //go to next slider
              insideSliders = false; //end of sliders
            }
            break;
          case ']':
            break;
          default:
            if (!insideQuotes || (qComma != knownMode)) break; //if not current mode, do nothing
            if ((!( (printedChars >= getCols()-1) || printedChars >= sizeof(lineBuffer)-1)) && !atSignFound)
              lineBuffer[printedChars++] = singleJsonSymbol;
            if (insideSliders && sliderCounter < 5 && printedCharsSliders < LINE_BUFFER_SIZE && insideSliders) {
              sliderNames[sliderCounter][printedCharsSliders++] = singleJsonSymbol;
            }
        }
        if ((qComma > knownMode)) break;
      }
      //if no atsign add 2 default
      if (isEffect && !atSignFound) {
        sprintf_P(sliderNames[sliderCounter++], sliderDefaults[0]);
        sprintf_P(sliderNames[sliderCounter++], sliderDefaults[1]);
      }
      //else if no slidernames then nothing

      for (;printedChars < getCols()-1 && printedChars < sizeof(lineBuffer)-1; printedChars++) lineBuffer[printedChars]=' '; //add spaces
      lineBuffer[printedChars] = 0; //end of string
      char* lineBufferPtr = lineBuffer;

      if (isEffect) {
        if (noteFound)
          lineBufferPtr = lineBuffer + 4; //WLEDSR: remove 2 spaces and the 2 of the 3 byte note characters (1 is skipped in the switch)
        for (uint8_t i = 0; i < sliderCounter; i++) {
          if (strlen_P(sliderNames[i]) > 0) {
            for (printedCharsSliders = strlen_P(sliderNames[i]);printedCharsSliders < getCols()-1 && printedCharsSliders < sizeof(sliderNames[i])-1; printedCharsSliders++) sliderNames[i][printedCharsSliders]=' '; //add spaces
            sliderNames[i][printedCharsSliders] = 0; //end of string
          }
          DEBUG_PRINT(i);
          DEBUG_PRINTLN(sliderNames[i]);
        }
      }
      DEBUG_PRINTLN(lineBufferPtr);
      drawString(1, row*lineHeight, lineBufferPtr);

      //WLEDSR: Add effect or palette icon
      if (isEffect) {
        if (noteFound)
     // DAA //        drawGlyph(0, row*lineHeight, 77, u8x8_font_open_iconic_play_1x1)
     ; // note effect
        else
      // DAA //       drawGlyph(0, row*lineHeight, 70, u8x8_font_open_iconic_thing_1x1)
      ; // normal effect
      }
      else {
      // DAA //     drawGlyph(0, row*lineHeight, 72, u8x8_font_open_iconic_thing_1x1)
      ; // pallette
      }
    }

    /**
     * If there screen is off or in clock is displayed,
     * this will return true. This allows us to throw away
     * the first input from the rotary encoder but
     * to wake up the screen.
     */
    bool wakeDisplay() {
      if (type == NONE || !enabled) return false;
      knownHour = 99;
      if (displayTurnedOff) {
        // Turn the display back on
        sleepOrClock(false);
        redraw(true);
        return true;
      }
      else {
        redraw(false); //WLEDSR: not always done in loop
        return false;
      }
    }

    /**
     * Allows you to show up to two lines as overlay for a
     * period of time.
     * Clears the screen and prints on the middle two lines.
     */
    void overlay(const char* line1, const char *line2, long showHowLong) {
      if (type == NONE || !enabled) return;

      if (displayTurnedOff) {
        // Turn the display back on (includes clear())
        sleepOrClock(false);
      } else {
        clear();
      }

      // Print the overlay
      if (line1) {
        String buf = line1;
        center(buf, getCols());
        drawString(0, 1*lineHeight, buf.c_str());
      }
      if (line2) {
        String buf = line2;
        center(buf, getCols());
        drawString(0, 2*lineHeight, buf.c_str());
      }
      overlayUntil = millis() + showHowLong;
    }


   
    void setLineType(byte lT) {
      lineType = (Line4Type) lT;
    }

    /**
     * Line 3 or 4 (last two lines) can be marked with an
     * arrow in the first column. Pass 2 or 3 to this to
     * specify which line to mark with an arrow.
     * Any other values are ignored.
     */
    void setMarkLine(byte newMarkLineNum) {
      if (newMarkLineNum == 2 || newMarkLineNum == 3) {
        markLineNum = newMarkLineNum;
      }
      else {
        markLineNum = 0;
      }
    }

    /**
     * Enable sleep (turn the display off) or clock mode.
     */
    void sleepOrClock(bool enabled) {
      clear();
      if (enabled) {
        if (clockMode) showTime();
        else           setPowerSave(1);
        displayTurnedOff = true;
      } else {
        setPowerSave(0);
        displayTurnedOff = false;
      }
    }

    /**
     * Display the current date and time in large characters
     * on the middle rows. Based 24 or 12 hour depending on
     * the useAMPM configuration.
     */
    void showTime(bool fullScreen = true) {
      if (type == NONE || !enabled) return;
      char lineBuffer[LINE_BUFFER_SIZE];

      updateLocalTime();
      byte minuteCurrent = minute(localTime);
      byte hourCurrent   = hour(localTime);
      byte secondCurrent = second(localTime);
      if (knownMinute == minuteCurrent && knownHour == hourCurrent) {
        // Time hasn't changed.
        if (!fullScreen) return;
      }
      knownMinute = minuteCurrent;
      knownHour = hourCurrent;

      byte currentMonth = month(localTime);
      sprintf_P(lineBuffer, PSTR("%s %2d "), monthShortStr(currentMonth), day(localTime));
      if (fullScreen)
        draw2x2String(DATE_INDENT, lineHeight==1 ? 0 : lineHeight, lineBuffer); // adjust for 8 line displays
      else
        drawString(1, lineHeight*3, lineBuffer);

      byte showHour = hourCurrent;
      boolean isAM = false;
      if (useAMPM) {
        if (showHour == 0) {
          showHour = 12;
          isAM = true;
        }
        else if (showHour > 12) {
          showHour -= 12;
          isAM = false;
        }
        else {
          isAM = true;
        }
      }

      sprintf_P(lineBuffer, (secondCurrent%2 || !fullScreen) ? PSTR("%2d:%02d") : PSTR("%2d %02d"), (useAMPM ? showHour : hourCurrent), minuteCurrent);
      // For time, we always use LINE_HEIGHT of 2 since
      // we are printing it big.
      if (fullScreen) {
        draw2x2String(TIME_INDENT+2, lineHeight*2, lineBuffer);
        sprintf_P(lineBuffer, PSTR("%02d"), secondCurrent);
        if (useAMPM) drawString(12+(fullScreen?0:2), lineHeight*2, (isAM ? "AM" : "PM"), true);
        else         drawString(12, lineHeight*2+1, lineBuffer, true); // even with double sized rows print seconds in 1 line
      } else {
        drawString(9+(useAMPM?0:2), lineHeight*3, lineBuffer);
        if (useAMPM) drawString(12+(fullScreen?0:2), lineHeight*3, (isAM ? "AM" : "PM"), true);
      }
    }


	
    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    //void addToJsonInfo(JsonObject& root) {
      //JsonObject user = root["u"];
      //if (user.isNull()) user = root.createNestedObject("u");
      //JsonArray data = user.createNestedArray(F("4LineDisplay"));
      //data.add(F("Loaded."));
    //}

    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    //void addToJsonState(JsonObject& root) {
    //}

    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    //void readFromJsonState(JsonObject& root) {
    //  if (!initDone) return;  // prevent crash on boot applyPreset()
    //}

    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     *
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     *
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     *
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root) {
      JsonObject top   = root.createNestedObject(FPSTR(_name));
      top[FPSTR(_enabled)]       = enabled;
//  DAA  //      JsonArray io_pin = top.createNestedArray("pin");
//  DAA  //      for (byte i=0; i<5; i++) io_pin.add(ioPin[i]);
//  DAA  //       top["help4Pins"]           = F("Clk,Data,CS,DC,RST"); // help for Settings page
      top["type"]                = type;
      top["help4Type"]           = F("1=SSD1306,2=SH1106,3=SSD1306_128x64,4=SSD1305,5=SSD1305_128x64,6=SSD1306_SPI,7=SSD1306_SPI_128x64"); // help for Settings page
      top[FPSTR(_flip)]          = (bool) flip;
      top[FPSTR(_contrast)]      = contrast;
      top[FPSTR(_refreshRate)]   = refreshRate/1000;
      top[FPSTR(_screenTimeOut)] = screenTimeout/1000;
      top[FPSTR(_sleepMode)]     = (bool) sleepMode;
      top[FPSTR(_clockMode)]     = (bool) clockMode;
      top[FPSTR(_forceAutoRedraw)]   = (bool) forceAutoRedraw;
      top[FPSTR(_noAutoRedraw)]      = (bool) noAutoRedraw;
      top[FPSTR(_busClkFrequency)] = ioFrequency/1000;
      DEBUG_PRINTLN(F("4 Line Display config saved."));
    }

    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
     *
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     */
    bool readFromConfig(JsonObject& root) {
      bool needsRedraw    = false;
      DisplayType newType = type;
 //  DAA  //      int8_t newPin[5]; for (byte i=0; i<5; i++) newPin[i] = ioPin[i];

      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) {
        DEBUG_PRINT(FPSTR(_name));
        DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
        return false;
      }

      enabled       = top[FPSTR(_enabled)] | enabled;
      newType       = top["type"] | newType;
//  DAA  //       for (byte i=0; i<5; i++) newPin[i] = top["pin"][i] | ioPin[i];
      flip          = top[FPSTR(_flip)] | flip;
      contrast      = top[FPSTR(_contrast)] | contrast;
      refreshRate   = (top[FPSTR(_refreshRate)] | refreshRate/1000) * 1000;
      screenTimeout = (top[FPSTR(_screenTimeOut)] | screenTimeout/1000) * 1000;
      sleepMode     = top[FPSTR(_sleepMode)] | sleepMode;
      clockMode     = top[FPSTR(_clockMode)] | clockMode;
      forceAutoRedraw   = top[FPSTR(_forceAutoRedraw)] | forceAutoRedraw;
      noAutoRedraw      = top[FPSTR(_noAutoRedraw)] | noAutoRedraw;
      ioFrequency   = min(3400, max(100, (int)(top[FPSTR(_busClkFrequency)] | ioFrequency/1000))) * 1000;  // limit frequency
      if (newType == SSD1306_SPI || newType == SSD1306_SPI64)
        ioFrequency = min(20000, max(500, (int)(top[FPSTR(_busClkFrequency)] | ioFrequency/1000))) * 1000;  // limit frequency
      else
        ioFrequency = min(3400, max(100, (int)(top[FPSTR(_busClkFrequency)] | ioFrequency/1000))) * 1000;  // limit frequency

      DEBUG_PRINT(FPSTR(_name));
      if (!initDone) {
        // first run: reading from cfg.json
 //  DAA  //        for (byte i=0; i<5; i++) ioPin[i] = newPin[i];
        type = newType;
        DEBUG_PRINTLN(F(" config loaded."));
      } else {
        DEBUG_PRINTLN(F(" config (re)loaded."));
        // changing parameters from settings page
        bool pinsChanged = false;
//  DAA  //         for (byte i=0; i<5; i++) if (ioPin[i] != newPin[i]) { pinsChanged = true; break; }
        if (pinsChanged || type!=newType) {
   // DAA //          if (type != NONE) delete u8x8;
 //  DAA  //          PinOwner po = PinOwner::UM_FourLineDisplay;
 //  DAA  //          if (ioPin[0]==HW_PIN_SCL && ioPin[1]==HW_PIN_SDA) po = PinOwner::HW_I2C;  // allow multiple allocations of HW I2C bus pins
 //  DAA  //         pinManager.deallocateMultiplePins((const uint8_t *)ioPin, (type == SSD1306_SPI || type == SSD1306_SPI64) ? 5 : 2, po);
 //  DAA  //          for (byte i=0; i<5; i++) ioPin[i] = newPin[i];
 //  DAA  //          if (ioPin[0]<0 || ioPin[1]<0) { // data & clock must be > -1
 //  DAA  //            type = NONE;
 //  DAA  //            enabled = false;
  //  DAA  //           return true;
 //  DAA  //          } else type = newType;
          setup();
          needsRedraw |= true;
        }
   // DAA //        if (!(type == SSD1306_SPI || type == SSD1306_SPI64)) u8x8->setBusClock(ioFrequency); // can be used for SPI too
        setContrast(contrast);
        setFlipMode(flip);
        if (needsRedraw && !wakeDisplay()) redraw(true);
      }
      // use "return !top["newestParameter"].isNull();" when updating Usermod with new features
      return !top[FPSTR(_enabled)].isNull();
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId() {
      return USERMOD_ID_FOUR_LINE_DISP;
    }
};


// strings to reduce flash memory usage (used more than twice)
const char FourLineDisplayUsermod::_name[]            PROGMEM = "4LineDisplay";
const char FourLineDisplayUsermod::_enabled[]         PROGMEM = "enabled";
const char FourLineDisplayUsermod::_contrast[]        PROGMEM = "contrast";
const char FourLineDisplayUsermod::_refreshRate[]     PROGMEM = "refreshRateSec";
const char FourLineDisplayUsermod::_screenTimeOut[]   PROGMEM = "screenTimeOutSec";
const char FourLineDisplayUsermod::_flip[]            PROGMEM = "flip";
const char FourLineDisplayUsermod::_sleepMode[]       PROGMEM = "sleepMode";
const char FourLineDisplayUsermod::_clockMode[]       PROGMEM = "clockMode";
const char FourLineDisplayUsermod::_forceAutoRedraw[] PROGMEM = "forceAutoRedraw (spi)";
const char FourLineDisplayUsermod::_noAutoRedraw[]    PROGMEM = "noAutoRedraw (i2c)";
const char FourLineDisplayUsermod::_busClkFrequency[] PROGMEM = "i2c-freq-kHz";

//  DAA  //// Turn off display after 5 minutes with no change.
//  DAA  //    if(!displayTurnedOff && millis() - lastRedraw > 5*60*1000)
//  DAA  //        {
//  DAA  //            digitalWrite(TFT_BL, LOW); // Turn backlight off. 
//  DAA  //            displayTurnedOff = true;
//  DAA  //        } 
//  DAA  //
//  DAA  //// Check if values which are shown on display changed from the last time.
//  DAA  //    if (((apActive) ? String(apSSID) : WiFi.SSID()) != knownSsid)
//  DAA  //    {
//  DAA  //    needRedraw = true;
//  DAA  //    }
//  DAA  //    else if (knownIp != (apActive ? IPAddress(4, 3, 2, 1) : WiFi.localIP()))
//  DAA  //    {
//  DAA  //    needRedraw = true;
//  DAA  //    }
//  DAA  //    else if (knownBrightness != bri)
//  DAA  //    {
//  DAA  //    needRedraw = true;
//  DAA  //    }
//  DAA  //    else if (knownMode != strip.getMainSegment().mode)
//  DAA  //    {
//  DAA  //    needRedraw = true;
//  DAA  //    }
//  DAA  //    else if (knownPalette != strip.getMainSegment().palette)
//  DAA  //    {
//  DAA  //    needRedraw = true;
//  DAA  //    }
//  DAA  //
//  DAA  //    if (!needRedraw)
//  DAA  //    {
//  DAA  //    return;
//  DAA  //    }
//  DAA  //    needRedraw = false;
//  DAA  //  
//  DAA  //    if (displayTurnedOff)
//  DAA  //    {
//  DAA  //        digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on.
//  DAA  //        displayTurnedOff = false;
//  DAA  //    }
//  DAA  //    lastRedraw = millis();
//  DAA  //
//  DAA  //// Update last known values.
//  DAA  //    #if defined(ESP8266)
//  DAA  //        knownSsid = apActive ? WiFi.softAPSSID() : WiFi.SSID();
//  DAA  //    #else
//  DAA  //        knownSsid = WiFi.SSID();
//  DAA  //    #endif
//  DAA  //    knownIp = apActive ? IPAddress(4, 3, 2, 1) : WiFi.localIP();
//  DAA  //    knownBrightness = bri;
//  DAA  //  knownMode = strip.getMainSegment().mode;
//  DAA  //  knownPalette = strip.getMainSegment().palette;
//  DAA  //
//  DAA  //    tft.fillScreen(TFT_BLACK);
//  DAA  //    tft.setTextSize(2);
//  DAA  //// First row with Wifi name
//  DAA  //    tft.setTextColor(TFT_SILVER);
//  DAA  //    tft.setCursor(3, 40);
//  DAA  //    tft.print(knownSsid.substring(0, tftcharwidth > 1 ? tftcharwidth - 1 : 0));
//  DAA  //// Print `~` char to indicate that SSID is longer, than our dicplay
//  DAA  //    if (knownSsid.length() > tftcharwidth)
//  DAA  //        tft.print("~");
//  DAA  //
//  DAA  //// Second row with AP IP and Password or IP
//  DAA  //    tft.setTextColor(TFT_GREEN);
//  DAA  //    tft.setTextSize(2);
//  DAA  //    tft.setCursor(1, 24);
//  DAA  //// Print AP IP and password in AP mode or knownIP if AP not active.
//  DAA  //
//  DAA  //    if (apActive)
//  DAA  //    {
//  DAA  //    tft.setTextColor(TFT_YELLOW);
//  DAA  //    tft.print("AP IP: ");
//  DAA  //    tft.print(knownIp);
//  DAA  //    tft.setCursor(1,46);
//  DAA  //    tft.setTextColor(TFT_YELLOW);
//  DAA  //    tft.print("AP Pass:");
//  DAA  //    tft.print(apPass);
//  DAA  //    }
//  DAA  //    else
//  DAA  //    {
//  DAA  //    tft.setTextColor(TFT_GREEN);
//  DAA  //    tft.print("IP: ");
//  DAA  //    tft.print(knownIp);
//  DAA  //    tft.setCursor(1,46);
//  DAA  //    //tft.print("Signal Strength: ");
//  DAA  //    //tft.print(i.wifi.signal);
//  DAA  //    tft.setTextColor(TFT_WHITE);
//  DAA  //    tft.print("Bri: ");
//  DAA  //    tft.print(((float(bri)/255)*100),0);
//  DAA  //    tft.print("%");
//  DAA  //    }
//  DAA  //
//  DAA  //// Third row with mode name
//  DAA  //    tft.setCursor(1, 68);
//  DAA  //    uint8_t qComma = 0;
//  DAA  //    bool insideQuotes = false;
//  DAA  //    uint8_t printedChars = 0;
//  DAA  //    char singleJsonSymbol;
//  DAA  //// Find the mode name in JSON
//  DAA  //    for (size_t i = 0; i < strlen_P(JSON_mode_names); i++)
//  DAA  //    {
//  DAA  //        singleJsonSymbol = pgm_read_byte_near(JSON_mode_names + i);
//  DAA  //        switch (singleJsonSymbol)
//  DAA  //        {
//  DAA  //            case '"':
//  DAA  //            insideQuotes = !insideQuotes;
//  DAA  //        break;
//  DAA  //        case '[':
//  DAA  //        case ']':
//  DAA  //        break;
//  DAA  //        case ',':
//  DAA  //        qComma++;
//  DAA  //        default:
//  DAA  //            if (!insideQuotes || (qComma != knownMode))
//  DAA  //            break;
//  DAA  //        tft.setTextColor(TFT_MAGENTA);
//  DAA  //        tft.print(singleJsonSymbol);
//  DAA  //        printedChars++;
//  DAA  //        }
//  DAA  //    if ((qComma > knownMode) || (printedChars > tftcharwidth - 1))
//  DAA  //      break;
//  DAA  //    }
//  DAA  //// Fourth row with palette name
//  DAA  //    tft.setTextColor(TFT_YELLOW);
//  DAA  //    tft.setCursor(1, 90);
//  DAA  //    qComma = 0;
//  DAA  //    insideQuotes = false;
//  DAA  //    printedChars = 0;
//  DAA  //// Looking for palette name in JSON.
//  DAA  //    for (size_t i = 0; i < strlen_P(JSON_palette_names); i++)
//  DAA  //    {
//  DAA  //    singleJsonSymbol = pgm_read_byte_near(JSON_palette_names + i);
//  DAA  //        switch (singleJsonSymbol)
//  DAA  //        {
//  DAA  //        case '"':
//  DAA  //        insideQuotes = !insideQuotes;
//  DAA  //        break;
//  DAA  //        case '[':
//  DAA  //        case ']':
//  DAA  //        break;
//  DAA  //        case ',':
//  DAA  //        qComma++;
//  DAA  //        default:
//  DAA  //            if (!insideQuotes || (qComma != knownPalette))
//  DAA  //            break;
//  DAA  //        tft.print(singleJsonSymbol);
//  DAA  //        printedChars++;
//  DAA  //        }
//  DAA  //// The following is modified from the code from the u8g2/u8g8 based code (knownPalette was knownMode)
//  DAA  //    if ((qComma > knownPalette) || (printedChars > tftcharwidth - 1))
//  DAA  //      break;
//  DAA  //    }
//  DAA  //// Fifth row with estimated mA usage
//  DAA  //    tft.setTextColor(TFT_SILVER);
//  DAA  //    tft.setCursor(1, 112);
//  DAA  //// Print estimated milliamp usage (must specify the LED type in LED prefs for this to be a reasonable estimate).
//  DAA  //    tft.print("Current: ");
//  DAA  //    tft.print(strip.currentMilliamps);
//  DAA  //    tft.print("mA");
//  DAA  //    }
//  DAA  //    /*
//  DAA  //     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
//  DAA  //     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
//  DAA  //     * Below it is shown how this could be used for e.g. a light sensor
//  DAA  //     */
//  DAA  //    /*
//  DAA  //    void addToJsonInfo(JsonObject& root)
//  DAA  //    {
//  DAA  //      int reading = 20;
//  DAA  //      //this code adds "u":{"Light":[20," lux"]} to the info object
//  DAA  //      JsonObject user = root["u"];
//  DAA  //      if (user.isNull()) user = root.createNestedObject("u");
//  DAA  //
//  DAA  //      JsonArray lightArr = user.createNestedArray("Light"); //name
//  DAA  //      lightArr.add(reading); //value
//  DAA  //      lightArr.add(" lux"); //unit
//  DAA  //    }
//  DAA  //    */
//  DAA  //
//  DAA  //
//  DAA  //    /*
//  DAA  //     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
//  DAA  //     * Values in the state object may be modified by connected clients
//  DAA  //     */
//  DAA  //    void addToJsonState(JsonObject& root)
//  DAA  //    {
//  DAA  //      //root["user0"] = userVar0;
//  DAA  //    }
//  DAA  //
//  DAA  //
//  DAA  //    /*
//  DAA  //     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
//  DAA  //     * Values in the state object may be modified by connected clients
//  DAA  //     */
//  DAA  //    void readFromJsonState(JsonObject& root)
//  DAA  //    {
//  DAA  //      userVar0 = root["user0"] | userVar0; //if "user0" key exists in JSON, update, else keep old value
//  DAA  //      //if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));
//  DAA  //    }
//  DAA  //
//  DAA  //
//  DAA  //    /*
//  DAA  //     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
//  DAA  //     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
//  DAA  //     * If you want to force saving the current state, use serializeConfig() in your loop().
//  DAA  //     *
//  DAA  //     * CAUTION: serializeConfig() will initiate a filesystem write operation.
//  DAA  //     * It might cause the LEDs to stutter and will cause flash wear if called too often.
//  DAA  //     * Use it sparingly and always in the loop, never in network callbacks!
//  DAA  //     *
//  DAA  //     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
//  DAA  //     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
//  DAA  //     *
//  DAA  //     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
//  DAA  //     */
//  DAA  //    void addToConfig(JsonObject& root)
//  DAA  //    {
//  DAA  //      JsonObject top = root.createNestedObject("exampleUsermod");
//  DAA  //      top["great"] = userVar0; //save this var persistently whenever settings are saved
//  DAA  //    }
//  DAA  //
//  DAA  //
//  DAA  //    /*
//  DAA  //     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
//  DAA  //     * This is called by WLED when settings are loaded (currently this only happens once immediately after boot)
//  DAA  //     *
//  DAA  //     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
//  DAA  //     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
//  DAA  //     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
//  DAA  //     */
//  DAA  //    bool readFromConfig(JsonObject& root)
//  DAA  //    {
//  DAA  //      JsonObject top = root["top"];
//  DAA  //      bool configComplete = !top.isNull();
//  DAA  //
//  DAA  //      configComplete &= getJsonValue(top["great"], userVar0);
//  DAA  //
//  DAA  //      return configComplete;
//  DAA  //    }
//  DAA  //
//  DAA  //
//  DAA  //    /*
//  DAA  //     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
//  DAA  //     * This could be used in the future for the system to determine whether your usermod is installed.
//  DAA  //     */
//  DAA  //    uint16_t getId()
//  DAA  //    {
//  DAA  //      return USERMOD_ID_ST7789_DISPLAY;
//  DAA  //    }
//  DAA  //
//  DAA  //   //More methods can be added in the future, this example will then be extended.
//  DAA  //   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
//  DAA  //};