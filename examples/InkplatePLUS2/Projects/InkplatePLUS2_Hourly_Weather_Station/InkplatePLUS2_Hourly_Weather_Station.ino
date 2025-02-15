/*
   InkplatePLUS2_Hourly_Weather_Station example for Soldered Inkplate PLUS2
   For this example you will need only USB-C cable and Inkplate PLUS2.
   Select "Soldered InkplatePLUS2" from Tools -> Board menu.
   Don't have "Soldered InkplatePLUS2" option? Follow our tutorial and add it:
   https://soldered.com/learn/add-inkplate-6-board-definition-to-arduino-ide/

   This example will show you how you can use Inkplate PLUS2 to display API data,
   e.g. OpenWeather public weather API for real time data. It shows the forecast
   weather for 4 hours. What happens here is basically ESP32 connects to WiFi and
   sends an API call and the server returns data in JSON format containing data
   about weather, then using the library ArduinoJson we extract icons and temperatures
   per hour from JSON data and show it on Inkplate PLUS2. After displaying the weather,
   ESP32 goes to sleep and wakes up every DELAY_MS milliseconds to show new weather
   (you can change the time interval).

   IMPORTANT:
   Make sure to change your desired city and wifi credentials below.
   Also have ArduinoJson installed in your Arduino libraries:
   https://github.com/bblanchon/ArduinoJson

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: https://forum.soldered.com/
   24 July 2023 by Soldered
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATEPLUS2
#error "Wrong board selection for this example, please select Inkplate PLUS2 in the boards menu."
#endif

//---------- CHANGE HERE  -------------:

// City name to be displayed on the bottom
char city[128] = "OSIJEK";

// Coordinates sent to the api
char lon[] = "18.5947808";
char lat[] = "45.5510548";

// Change to your wifi ssid and password
char ssid[] = "";
char pass[] = "";

// Change to your api key, if you don't have one, head over to:
// https://openweathermap.org/guide , register and copy the key provided
char apiKey[] = "";

// Uncomment this for MPH and Fahrenheit output, also uncomment it in the begining of Network.cpp
// #define AMERICAN

//----------------------------------

// Include Inkplate library to the sketch
#include "Inkplate.h"

// Header file for easier code readability
#include "Network.h"

// Including fonts used
#include "Fonts/Inter12pt7b.h"
#include "Fonts/Inter30pt7b.h"
#include "Fonts/Inter8pt7b.h"

// Including icons generated by the py file
#include "icons.h"

// Delay between API calls, about 1000 per month, which is the free tier limit
#define DELAY_MS 267800L

// Inkplate object
Inkplate display(INKPLATE_1BIT);

// All our network functions are in this object, see Network.h
Network network;

// Constants used for drawing icons
char abbrs[32][32] = {"01d", "02d", "03d", "04d", "09d", "10d", "11d", "13d", "50d",
                      "01n", "02n", "03n", "04n", "09n", "10n", "11n", "13n", "50n"};

const uint8_t *logos[18] = {
    icon_01d, icon_02d, icon_03d, icon_04d, icon_09d, icon_10d, icon_11d, icon_13d, icon_50d,
    icon_01n, icon_02n, icon_03n, icon_04n, icon_09n, icon_10n, icon_11n, icon_13n, icon_50n,
};

const uint8_t *s_logos[18] = {
    icon_s_01d, icon_s_02d, icon_s_03d, icon_s_04d, icon_s_09d, icon_s_10d, icon_s_11d, icon_s_13d, icon_s_50d,
    icon_s_01n, icon_s_02n, icon_s_03n, icon_s_04n, icon_s_09n, icon_s_10n, icon_s_11n, icon_s_13n, icon_s_50n,
};

// Variables for storing temperature
char temps[4][8] = {
    "-",
    "-",
    "-",
    "-",
};

// Variables for storing hour strings
char hours[4][8] = {
    "",
    "",
    "",
    "",
};

// Variables for storing current time and weather info
char currentTemp[16] = "-";
char currentWind[16] = "-";

char currentTime[16] = "--:--";

int timeZone;

char currentWeather[32] = "-";
char currentWeatherAbbr[8] = "01d";

char abbr1[16];
char abbr2[16];
char abbr3[16];
char abbr4[16];

// functions defined below
void drawWeather();
void drawCurrent();
void drawTemps();
void drawCity();
void drawTime();

void setup()
{
    // Begin serial and display
    Serial.begin(115200);
    display.begin();

    // Calling our begin from network.h file
    network.begin(ssid, pass);

    // Get all relevant data, see Network.cpp for info
    Serial.print("Fetching data");
    while (!network.getData(lat, lon, apiKey, temps[0], temps[1], temps[2], temps[3], currentTemp, currentWind,
                            currentTime, currentWeather, currentWeatherAbbr, abbr1, abbr2, abbr3, abbr4, &timeZone))
    {
        Serial.print('.');
        delay(500);
    }
    Serial.println();
    network.getTime(currentTime, timeZone);
    network.getHours(hours[0], hours[1], hours[2], hours[3], timeZone);

    // Draw data, see functions below for info
    Serial.println("Drawing on the screen");
    drawWeather();
    drawCurrent();
    drawTemps();
    drawCity();
    drawTime();

    // Next line actually draw all on the screen
    display.display();

    // Go to sleep before checking again
    Serial.println("Going to sleep, bye!");
    esp_sleep_enable_timer_wakeup(1000L * DELAY_MS); // Activate wake-up timer
    (void)esp_deep_sleep_start(); // Start deep sleep (this function does not return). Program stops here.
}

void loop()
{
    // Never here! If you are using deep sleep, the whole program should be in setup() because the board restarts each
    // time. loop() must be empty!
}

// Function for drawing weather info
void drawWeather()
{
    // Searching for weather state abbreviation
    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbrs[i], currentWeatherAbbr) == 0)
            display.drawBitmap(50, 75, logos[i], 92, 92, BLACK);
    }

    // Draw weather state
    display.setFont(&Inter12pt7b);
    display.setTextSize(1);
    display.setCursor(35, 200);
    display.println(currentWeather);
}

// Function for drawing current time
void drawTime()
{
    // Drawing current time
    display.setFont(&Inter12pt7b);
    display.setTextSize(1);

    display.setCursor(E_INK_WIDTH - 10 - 14 * strlen(currentTime), 30);
    display.println(currentTime);
}

// Function for drawing city name
void drawCity()
{
    // Drawing city name
    display.setFont(&Inter12pt7b);
    display.setTextSize(2);

    display.setCursor(300 - 14 * strlen(city), 565);
    display.println(city);
}

// Draw celsius degrees if AMERICAN isn't defined or Fahrenheit degrees if it's defined
void drawTempUnit()
{
#ifdef AMERICAN
    display.println(F("F"));
#else
    display.println(F("C"));
#endif
}

// Function for drawing temperatures
void drawTemps()
{
    // Drawing 4 rectangles in which temperatures will be written
    int rectWidth = 115;
    int rectSpacing = (E_INK_WIDTH - rectWidth * 4) / 5;
    int yRectangleOffset = 250;
    int rectHeight = 260;

    for (int i = 0; i < 4; i++)
    {
        display.drawRect((i + 1) * rectSpacing + i * rectWidth, yRectangleOffset, rectWidth, rectHeight,
                         BLACK);
    }

    int textMargin = 10;

    display.setFont(&Inter8pt7b);
    display.setTextSize(2);

    int hourOffset = 20;

    // Drawing hours into rectangles
    for (int i = 0; i < 4; i++)
    {
        display.setCursor((i + 1) * rectSpacing + i * rectWidth + textMargin,
                          yRectangleOffset + textMargin + hourOffset);
        display.println(hours[i]);
    }

    // Drawing temperature values into rectangles
    display.setFont(&Inter12pt7b);
    display.setTextSize(2);

    int tempOffset = 105;

    for (int i = 0; i < 4; i++)
    {
        display.setCursor((i + 1) * rectSpacing + i * rectWidth + textMargin,
                          yRectangleOffset + textMargin + tempOffset);
        display.print(temps[i]);
        drawTempUnit();
    }

    int iconOffset = 130;

    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbr1, abbrs[i]) == 0)
            display.drawBitmap(1 * rectSpacing + 0 * rectWidth + textMargin, yRectangleOffset + textMargin + iconOffset,
                               logos[i], 96, 96, BLACK, WHITE);
    }

    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbr2, abbrs[i]) == 0)
            display.drawBitmap(2 * rectSpacing + 1 * rectWidth + textMargin, yRectangleOffset + textMargin + iconOffset,
                               logos[i], 96, 96, BLACK, WHITE);
    }

    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbr3, abbrs[i]) == 0)
            display.drawBitmap(3 * rectSpacing + 2 * rectWidth + textMargin, yRectangleOffset + textMargin + iconOffset,
                               logos[i], 96, 96, BLACK, WHITE);
    }

    for (int i = 0; i < 18; ++i)
    {
        // If found draw specified icon
        if (strcmp(abbr4, abbrs[i]) == 0)
            display.drawBitmap(4 * rectSpacing + 3 * rectWidth + textMargin, yRectangleOffset + textMargin + iconOffset,
                               logos[i], 96, 96, BLACK, WHITE);
    }
}

// Current weather drawing function
void drawCurrent()
{
    // Drawing current information

    // Temperature:
    display.setFont(&Inter30pt7b);
    display.setTextSize(2);

    display.setCursor(180, 160);
    display.print(currentTemp);

    int x = display.getCursorX();
    int y = display.getCursorY();

    display.setFont(&Inter12pt7b);
    display.setTextSize(2);

    display.setCursor(x, y);
    drawTempUnit();

    // Wind:
    display.setFont(&Inter30pt7b);
    display.setTextSize(2);

    display.setCursor(360, 160);
    display.print(currentWind);

    x = display.getCursorX();
    y = display.getCursorY();

    display.setFont(&Inter12pt7b);
    display.setTextSize(2);

    display.setCursor(x, y);

#ifdef AMERICAN
    display.println(F("mph"));
#else
    display.println(F("m/s"));
#endif

    // Labels underneath
    display.setFont(&Inter12pt7b);
    display.setTextSize(1);

    display.setCursor(180, 200);
    display.println(F("TEMPERATURE"));

    display.setCursor(380, 200);
    display.println(F("WIND SPEED"));
}
