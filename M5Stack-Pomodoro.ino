/*********************************************
 * Author Luca Nardelli (lunard@gmail.com)
 * 
 * Description: the project aims to create a funny (and I hope usefult) pomodoro with the M5Stack Dev Kit
 ********************************************/

#include <Free_Fonts.h>
#include <M5Display.h>
#include <M5Stack.h>
#include <WiFiHelper.h>


void setup(void) {

  M5.begin();

  // Setup logo
  M5.Lcd.setBrightness(200);
  M5.Lcd.drawJpgFile(SD, "/pomodoro.jpg");
}

void loop() {

}