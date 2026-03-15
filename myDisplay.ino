#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>
#include <SPI.h>

#define TFT_CS 1
#define TFT_RST 2
#define TFT_DC 3
#define TFT_SCLK 4
#define TFT_MOSI 5
#define BTN_PLAY_PAUSE 6
#define BTN_NEXT       7
#define BTN_VOL_UP     8
#define BTN_VOL_DOWN   10
#define BTN_MUTE    0

int currentVolume = 50; // Local tracker for volume
char* SSID = "SSID";
const char* PASSWORD = "WIFI_PASSWORD";
const char* CLIENT_ID = "SPOTIFY_ID";
const char* CLIENT_SECRET = "SPOTIFY_CLIENT_SECRET";
unsigned long lastSpotifyCheck = 0; 
const unsigned long spotifyInterval = 1000; // 1 second

String lastArtist;
String lastTrackname;

Spotify sp(CLIENT_ID, CLIENT_SECRET);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);


void setup() {
    Serial.begin(115200);
    pinMode(BTN_PLAY_PAUSE, INPUT_PULLUP);
    pinMode(BTN_NEXT, INPUT_PULLUP);
    pinMode(BTN_VOL_UP, INPUT_PULLUP);
    pinMode(BTN_VOL_DOWN, INPUT_PULLUP);
    pinMode(BTN_MUTE, INPUT_PULLUP);
    tft.initR(INITR_BLACKTAB); // the type of screen
    tft.setRotation(1); // this makes the screen landscape! remove this line for portrait
    Serial.println("TFT Initialized!");
    tft.fillScreen(ST77XX_BLACK); // make sure there is nothing in the buffer

    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi...");
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.printf("\nConnected!\n");

    tft.setCursor(0,0); // make the cursor at the top left
    tft.write(WiFi.localIP().toString().c_str()); // print out IP on the screen

    sp.begin();
    while(!sp.is_auth()){
        sp.handle_client();
    }
    Serial.println("Authenticated");
}

void loop()
{
    // Only check Spotify if 1 second has passed
    if (millis() - lastSpotifyCheck >= spotifyInterval) {
        lastSpotifyCheck = millis(); // Reset the timer

        String currentArtist = sp.current_artist_names();
        String currentTrackname = sp.current_track_name();

        // Check if the artist changed
        if (lastArtist != currentArtist && currentArtist != "Something went wrong" && !currentArtist.isEmpty()) {
            tft.fillScreen(ST77XX_BLACK); 
            lastArtist = currentArtist;
            tft.setCursor(10, 10);
            tft.setTextColor(ST77XX_WHITE);
            tft.setTextSize(1);
            tft.print(lastArtist.c_str());
        }

        // Check if the track changed
        if (lastTrackname != currentTrackname && currentTrackname != "Something went wrong" && currentTrackname != "null") {
            tft.fillRect(0, 40, 160, 30, ST77XX_BLACK); 
            lastTrackname = currentTrackname;
            tft.setCursor(10, 40);
            tft.setTextColor(ST77XX_CYAN);
            tft.print(lastTrackname.c_str());
        }
    }

    // --- Button logic stays OUTSIDE the if-block ---
    // This allows buttons to remain responsive regardless of the 1-second timer.

    if (digitalRead(BTN_PLAY_PAUSE) == LOW) {
        sp.start_resume_playback(); 
        delay(300); // Simple debounce
    }

    // 2. Next Track
    if (digitalRead(BTN_NEXT) == LOW) {
        sp.skip();
        delay(300);
    }

    //3. Previous track
    if (digitalRead(BTN_NEXT) == LOW) {
        sp.previous();
        delay(300);
    }

    // 4. Volume Control
    if (digitalRead(BTN_VOL_UP) == LOW) {
        currentVolume = min(100, currentVolume + 10);
        sp.set_volume(currentVolume);
        Serial.printf("Volume: %d\n", currentVolume);
        delay(200);
    }

    if (digitalRead(BTN_VOL_DOWN) == LOW) {
        currentVolume = max(0, currentVolume - 10);
        sp.set_volume(currentVolume);
        Serial.printf("Volume: %d\n", currentVolume);
        delay(200);
    }
        if (digitalRead(BTN_MUTE) == LOW) {
        currentVolume = max(0, currentVolume - currentVolume);
        sp.set_volume(currentVolume);
        Serial.printf("Volume: %d\n", currentVolume);
        delay(200);
    }
}
