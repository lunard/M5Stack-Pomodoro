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
int accY_Treshhold = 0.4;
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
  CheckOrientation();
}

int CheckOrientation()
{
  long currentMillis = millis();
  if (lastRefreshOrientation == 0 || (currentMillis - lastRefreshOrientation) > refreshOrientationTimeout)
  {
    lastRefreshOrientation = currentMillis;

    updateIMUData();

    M5.Lcd.setCursor(0, 20);
    if (accY > accY_Treshhold)
    {
      M5.Lcd.println("clockwise");
    }
    else if (accY < -accY_Treshhold)
    {
      M5.Lcd.println("r-clockwise");
    }else{
      M5.Lcd.println("No rotation");
    }
  }
  else
    return -1;
}

int updateIMUData()
{
  M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
  M5.IMU.getAccelData(&accX, &accY, &accZ);
  M5.IMU.getAhrsData(&pitch, &roll, &yaw);
  M5.IMU.getTempData(&temp);

}