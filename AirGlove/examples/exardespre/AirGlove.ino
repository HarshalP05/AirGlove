#include <AirGlove.h>



#define LED_PIN 2
#define LEFT_CLICK_PIN 4
#define RIGHT_CLICK_PIN 12
#define LASER_PIN 19
#define LASER_BUTTON_PIN 13
#define MENU_BUTTON_PIN 32  
#define MENU_SELECT_PIN 27  
#define BATTERY_PIN 34

AirGlove airGlove(LED_PIN, LEFT_CLICK_PIN, RIGHT_CLICK_PIN, LASER_PIN, LASER_BUTTON_PIN, MENU_BUTTON_PIN, MENU_SELECT_PIN);

void setup() {
    airGlove.AirGlove_init();
    airGlove.calibrateMPU();
  
}

void loop() {
    airGlove.readBatteryPercentage();
    airGlove.moveCursor();  // Runs only if in NORMAL, GAMING, or PPT mode
    airGlove.leftClick();
    airGlove.rightClick();
    airGlove.controlLaser();
    //airGlove.displayModeSelection();  // Show main menu at startup
    //airGlove.handleButtonPress();
    //airGlove.handleSelection();
    airGlove.checkConnection();
  
    
}
