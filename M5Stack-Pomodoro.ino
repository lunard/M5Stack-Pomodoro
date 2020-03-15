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

#define ORIENTATION_UP 1
#define ORIENTATION_DOWN 2
#define ORIENTATION_LEFT 3
#define ORIENTATION_RIGHT 4

void setup(void)
{

  M5.begin();

  /*
    Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project
  */
  M5.Power.begin();

  M5.IMU.Init();

  // Setup logo
  M5.Lcd.setBrightness(100);
  M5.Lcd.drawJpgFile(SD, "/pomodoro.jpg");
  delay(1000);

  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2);
}

void loop()
{
  int orientation = CheckOrientation();
  RotateDisplayByOrientation(orientation);
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
  if (!orientation)
    return;
    
  switch (orientation)
  {
  case ORIENTATION_UP:
    M5.Lcd.println("UP");
    break;
  case ORIENTATION_RIGHT:
    M5.Lcd.println("RIGHT");
    break;
  case ORIENTATION_DOWN:
    M5.Lcd.println("DOWN");
    break;
  case ORIENTATION_LEFT:
    M5.Lcd.println("LEFT");
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