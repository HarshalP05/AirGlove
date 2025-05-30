#ifndef AIRGLOVE_H
#define AIRGLOVE_H

#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <BleMouse.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Kalman.h>


class AirGlove {
    private:
        const int led_pin;
        const int L_click_pin;
        const int R_click_pin;
        const int LASER_PIN;
        const int LASER_BUTTON_PIN;
        const int MENU_BUTTON_PIN;
        const int MENU_SELECT_PIN; 
        Kalman kalmanX;
        Kalman kalmanY;
        int gyroOffsetX;
        int gyroOffsetY;
        int gyroOffsetZ;
        int y;
        int rectWidth;
        int gapHeight;
        float percentage;
        MPU6050 mpu;
        BleMouse bleMouse;
        Adafruit_SSD1306 display;
        bool lastLaserButtonState;
        unsigned long lastOledUpdate = 0;
    public:
        AirGlove(int led, int leftClick, int rightClick,int LASER_PIN, int LASER_BUTTON_PIN,int menuButtonPin,int menuSelectPin);
        void AirGlove_init();
        void calibrateMPU();
        void moveCursor();
        void leftClick();
        void rightClick();
        void checkConnection();
        void controlLaser();
        void readBatteryPercentage();
        void show(int x, int y, int size, const char* text);
        void drawSmileyFace(int x, int y);
        void displayWelcomeSequence();
        void displayModeSelection();
        void handleSelection();
        void handleButtonPress();
        void  drawBatteryStatus(); 



};

#endif
