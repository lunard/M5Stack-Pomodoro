/*********************************************
 * Author Luca Nardelli (lunard@gmail.com)
 * 
 * Description: the project aims to create a funny (and I hope usefult) pomodoro with the M5Stack Dev Kit
 ********************************************/

// define must ahead #include <M5Stack.h>
#define M5STACK_MPU6886

#include <Free_Fonts.h>
#include <M5Display.h>
#include <M5Stack.h>
#include <WiFiHelper.h>
#include <ArduinoJson.h>

#include <SD.h>

// IMU data
// Global variables
float IMUTreshhold = 0.85;
float IMUTreshhold2 = 0.3;

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;
float pitch = 0.0F;
float roll = 0.0F;
float yaw = 0.0F;
float temp = 0.0F;

// refresh timer
unsigned long refreshOrientationTimeout = 500;
unsigned long lastRefreshOrientation = 0;
int lastOrientation = -1;

#define ORIENTATION_UP 1
#define ORIENTATION_DOWN 2
#define ORIENTATION_LEFT 3
#define ORIENTATION_RIGHT 4

#define TICK_CONTEINER_HEIGHT_IN_PX 190
#define TICK_CONTEINER_WIDTH_IN_PX 160

int tickDurationInSec = 2;
int currentTick = 0;
unsigned long lastTickCheck = 0;

unsigned long remoteSplashImgDownloadTimeoutInSec = 60 * 60 * 24;

#define NUMBER_OF_TICKS 10 // a tick is a single interval of timer
int tickHeight = (TICK_CONTEINER_HEIGHT_IN_PX / NUMBER_OF_TICKS) - 2;

struct Config
{
  char wifiSSID[64];
  char wifiPassword[64];
};
Config config;

WiFiHelper *wifiHelper;
IPAddress ipAddress;

void setup(void)
{

  M5.begin();

  Serial.begin(9600);

  /*
    Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project
  */
  M5.Power.begin();

  M5.IMU.Init();

  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setFreeFont(FS9);

  if (!loadConfiguraton("/config.json", config))
  {
    M5.Lcd.println("Cannot read the configuration!");
    Serial.println("Cannot read the configuration!");
  }

  wifiHelper = new WiFiHelper(config.wifiSSID, config.wifiPassword);

  // Setup logo
  M5.Lcd.setBrightness(100);
  M5.Lcd.drawJpgFile(SD, "/pomodoro.jpg");
  delay(500);

  M5.Lcd.clear(BLACK);
  M5.Lcd.drawPngFile(SD, "/wifi.png", 90, 70);
  M5.Lcd.setCursor(10, 20);
  M5.Lcd.print("Connecting to '" + String(config.wifiSSID) + "'..");

  ipAddress = wifiHelper->connect();
  M5.Lcd.setCursor(10, 50);
  M5.Lcd.print(ipAddress);
  M5.Lcd.print(" - ");
  M5.Lcd.print(wifiHelper->getFormattedTime());

  unsigned long lasEpochTime = readEpochTimeFromFile("/lastSplashImagesUpdate.json");
  delay(1000);

  if (lasEpochTime <= 0 || wifiHelper->getEpochTime() - lasEpochTime > remoteSplashImgDownloadTimeoutInSec)
  {
    M5.Lcd.clear(BLACK);
    M5.Lcd.setCursor(10, 70);
    M5.Lcd.print("Splash images to old!");
    M5.Lcd.setCursor(10, 95);
    M5.Lcd.print("..they will be downloaded again!");
    delay(3000);

    // Download new set of splash images
    int quantity = 10;
    downloadSplashImages("cats", quantity);
    for (size_t i = 0; i < quantity; i++)
    {
      String fileName = "/splashImage_" + String(i + 1) + ".jpg";
      M5.Lcd.drawJpgFile(SD, fileName.c_str());
      delay(2000);
    }
    writeEpochTimeToFile("/lastSplashImagesUpdate.json", wifiHelper->getEpochTime());
  }
}

void loop()
{
  int orientation = CheckOrientation();

  if (orientation > 0 && lastOrientation != orientation)
    M5.Lcd.clear(BLACK);

  RotateDisplayByOrientation(orientation);
  //TestOrientation(orientation);

  // check tick
  long currentMillis = millis();
  if (currentTick == 0 || currentMillis - lastTickCheck > tickDurationInSec * 1000)
  {
    currentTick++;
    lastTickCheck = currentMillis;
  }

  drawPage(orientation);

  if (orientation > 0)
    lastOrientation = orientation;
}

void drawPage(int orientation)
{

  if (currentTick > NUMBER_OF_TICKS)
  {
    // manage elapsed pomodoro!
  }
  else
  {
    if (orientation == ORIENTATION_UP)
    {
      drawTicks();
    }
  }

  if (orientation < 0 || lastOrientation == orientation)
    return;

  switch (orientation)
  {
  case ORIENTATION_UP:
    drawTicksContainer();
    break;
  case ORIENTATION_RIGHT:
    break;
  case ORIENTATION_DOWN:
    drawTicksContainer();
    break;
  case ORIENTATION_LEFT:
    break;
  default:
    break;
  }
}

void drawTicks()
{
  uint16_t color = 0;
  for (size_t i = 0; i <= currentTick; i++)
  {
    if (i <= 2)
      color = 0x1d69;
    if (i > 2 && i <= 4)
      color = 0x9d6a;
    if (i > 4 && i <= 6)
      color = 0xe66a;
    if (i > 6 && i <= 8)
      color = 0xe50a;
    if (i > 8 && i <= 10)
      color = 0xe06a;
    M5.Lcd.fillRect(81, 231 - (tickHeight * (i + 1)), TICK_CONTEINER_WIDTH_IN_PX - 1, tickHeight - 1, color);
  }
}

int CheckOrientation()
{
  long currentMillis = millis();
  if (lastRefreshOrientation == 0 || (currentMillis - lastRefreshOrientation) > refreshOrientationTimeout)
  {
    lastRefreshOrientation = currentMillis;

    updateIMUData();

    // down      accy < -0.85 & abs(accx) < 0.2
    // Up        accy > 0.85  & abs(accx) < 0.2
    // left      accx < -0.85 & abs(accy) < 0.2
    // right     accx > 0.85  & abs(accy) < 0.2

    if (accY < -IMUTreshhold && abs(accX) < IMUTreshhold2)
    {
      return ORIENTATION_DOWN;
    }
    else if (accY > IMUTreshhold && abs(accX) < IMUTreshhold2)
    {
      return ORIENTATION_UP;
    }
    else if (accX < -IMUTreshhold && abs(accY) < IMUTreshhold2)
    {
      return ORIENTATION_LEFT;
    }
    else if (accX > IMUTreshhold && abs(accY) < IMUTreshhold2)
    {
      return ORIENTATION_RIGHT;
    }
  }
  else
    return -1;
}

void RotateDisplayByOrientation(int orientation)
{
  if (orientation < 0)
    return;

  switch (orientation)
  {
  case ORIENTATION_UP:
    M5.Lcd.setRotation(1);
    break;
  case ORIENTATION_RIGHT:
    M5.Lcd.setRotation(2);
    break;
  case ORIENTATION_DOWN:
    M5.Lcd.setRotation(3);
    break;
  case ORIENTATION_LEFT:
    M5.Lcd.setRotation(0);
    break;
  default:
    break;
  }
}

void TestOrientation(int orientation)
{
  if (orientation < 0 || lastOrientation == orientation)
    return;

  M5.Lcd.clear(WHITE);
  switch (orientation)
  {
  case ORIENTATION_UP:
    M5.Lcd.drawPngFile(SD, "/up.png");
    break;
  case ORIENTATION_RIGHT:
    M5.Lcd.drawPngFile(SD, "/left.png");
    break;
  case ORIENTATION_DOWN:
    M5.Lcd.drawPngFile(SD, "/down.png");
    break;
  case ORIENTATION_LEFT:
    M5.Lcd.drawPngFile(SD, "/right.png");
    break;
  default:
    break;
  }
}

int updateIMUData()
{
  M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
  M5.IMU.getAccelData(&accX, &accY, &accZ);
  M5.IMU.getAhrsData(&pitch, &roll, &yaw);
  M5.IMU.getTempData(&temp);
}

void printIMUStatus()
{
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("%6.2f  %6.2f  %6.2f      ", gyroX, gyroY, gyroZ);
  M5.Lcd.setCursor(220, 42);
  M5.Lcd.print(" o/s");
  M5.Lcd.setCursor(0, 65);
  M5.Lcd.printf(" %5.2f   %5.2f   %5.2f   ", accX, accY, accZ);
  M5.Lcd.setCursor(220, 87);
  M5.Lcd.print(" G");
  M5.Lcd.setCursor(0, 110);
  M5.Lcd.printf(" %5.2f   %5.2f   %5.2f   ", pitch, roll, yaw);
  M5.Lcd.setCursor(220, 132);
  M5.Lcd.print(" degree");
  M5.Lcd.setCursor(0, 155);
  M5.Lcd.printf("Temperature : %.2f C", temp);
}

void drawTicksContainer()
{
  M5.Lcd.drawFastHLine(80, 230, TICK_CONTEINER_WIDTH_IN_PX, WHITE);
  M5.Lcd.drawFastVLine(80, 40, TICK_CONTEINER_HEIGHT_IN_PX, WHITE);
  M5.Lcd.drawFastVLine(240, 40, TICK_CONTEINER_HEIGHT_IN_PX, WHITE);
}

void downloadSplashImages(String topic, int quantity)
{

  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  int y = 130;
  String progressFileName = "/catProgress.png";
  for (size_t i = 0; i < quantity; i++)
  {
    M5.Lcd.fillRect(0, 0, 320, 90, BLACK); // clear
    M5.Lcd.setCursor(10, 60);
    if (i > 0)
    {
      M5.Lcd.print("Got splash " + String(i) + "/" + String(quantity) + "   ");
    }
    else
    {
      M5.Lcd.print("Getting first splash..");
    }

    String url = "https://source.unsplash.com/random/320x240?" + topic;
    String fileName = "/splashImage_" + String(i + 1) + ".jpg";

    bool result = wifiHelper->downloadFile(SD, url, fileName);
    Serial.println("Download of splash image " + fileName + (result ? " DONE" : " FAIL"));

    if (i > 1 && i % 8 == 0)
    {
      progressFileName = progressFileName.indexOf("Rotated") > 0 ? "/catProgress.png" : "/catProgressRotated.png";
    }

    M5.Lcd.drawPngFile(SD, progressFileName.c_str(), (i % 8) * 35 + 2, y, 35, 35);
  }
}

// See https://arduinojson.org/v6/doc/
bool loadConfiguraton(const char *filename, Config &config)
{
  if (!SD.exists(filename))
    return false;

  // Open file for reading
  File file = SD.open(filename);

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  //config.wifiPassword = doc["wifiPassword"];
  strlcpy(config.wifiSSID,              // <- destination
          doc["wifiSSID"],              // <- source
          sizeof(config.wifiSSID));     // <- destination's capacity
  strlcpy(config.wifiPassword,          // <- destination
          doc["wifiPassword"],          // <- source
          sizeof(config.wifiPassword)); // <- destination's capacity

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();

  return true;
}

bool writeEpochTimeToFile(const char *filename, unsigned long epochTime)
{
  if (SD.exists(filename))
    SD.remove(filename);

  // create file
  File file = SD.open(filename, FILE_WRITE);

  StaticJsonDocument<512> doc;
  doc["epochTime"] = epochTime;

  size_t byteWritten = serializeJsonPretty(doc, file);
  file.flush();
  file.close();
  if (byteWritten <= 0)
  {
    Serial.println("writeEpochTimeToFile: cannot write on the file: " + String(filename));
    return false;
  }
  else
    return true;
}

unsigned long readEpochTimeFromFile(const char *filename)
{
  if (!SD.exists(filename))
    return -1;

  // Open file for reading
  File file = SD.open(filename);

  StaticJsonDocument<512> doc;

  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println("readEpochTimeFromFile: failed to read file:" + String(filename));

  file.close();

  return doc["epochTime"].as<unsigned long>();
}