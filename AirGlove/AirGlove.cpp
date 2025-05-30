#include "AirGlove.h"
#include <Wire.h>


#define MAX_VOLTAGE 4.2
#define MIN_VOLTAGE 3.2
#define BATTERY_PIN 34
#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 64  
#define OLED_RESET    -1 
    // initilize as airglove
    AirGlove::Air Glove(int led, int leftClick, int rightClick, int laserPin, int laserButtonPin, int menuButtonPin, int menuSelectPin)
    : led_pin(led), L_click_pin(leftClick), R_click_pin(rightClick),
      LASER_PIN(laserPin), LASER_BUTTON_PIN(laserButtonPin),MENU_BUTTON_PIN(menuButtonPin),MENU_SELECT_PIN(menuSelectPin),
      gyroOffsetX(0), gyroOffsetY(0), gyroOffsetZ(0),percentage(0),lastLaserButtonState(HIGH),y(15),rectWidth(10), gapHeight(10),
      display(128, 64, &Wire, -1) {}
 
    
    void AirGlove::AirGlove_init() {
        Serial.begin(115200);
        Wire.begin(21, 22);
        bleMouse.begin();
        pinMode(led_pin, OUTPUT);
        pinMode(LASER_PIN, OUTPUT);
        pinMode(LASER_BUTTON_PIN, INPUT_PULLUP);
        pinMode(BATTERY_PIN, INPUT);
        pinMode(MENU_BUTTON_PIN, INPUT_PULLUP);
        pinMode(MENU_SELECT_PIN, INPUT_PULLUP);
        digitalWrite(LASER_PIN, LOW);  
    
        if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            Serial.println("SSD1306 allocation failed");
            while (1);
        }
        
        display.clearDisplay();
        displayWelcomeSequence();
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println("Initializing MPU6050...");
        display.display();
    
        Serial.println("Initializing MPU6050...");
        mpu.initialize();
    
        if (mpu.testConnection()) {
            Serial.println("MPU6050 connection successful!");
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("MPU6050 Ready!");
            display.display();
            delay(1000);
        } else {
            Serial.println("MPU6050 connection failed!");
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("MPU6050 FAIL!");
            display.display();
            while (1);
        }
    }
    float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
      }
      

 void AirGlove::readBatteryPercentage() {
          static unsigned long lastBatteryUpdate = 0;  
          if (millis() - lastBatteryUpdate < 5000) return;  
          lastBatteryUpdate = millis();
      
          int raw = analogRead(BATTERY_PIN);
          float voltage = raw * (3.3 / 4095.0) * 2;  
      
          percentage = mapfloat(voltage, 2.8, 4.2, 0, 100); 
      
          if (percentage >= 100) percentage = 100;
          if (percentage <= 0) percentage = 1;
          bleMouse.setBatteryLevel(percentage);
          Serial.print("Analog Value = ");
          Serial.print(raw);
          Serial.print("\t Voltage = ");
          Serial.print(voltage);
          Serial.print("V\t Battery = ");
          Serial.print(percentage);
          Serial.println("%");
          if (millis() - lastOledUpdate > 5000) {  
            lastOledUpdate = millis();
    
            drawBatteryStatus(); 
            display.display();
        }
      }
      


    
    
    void AirGlove::calibrateMPU() {
        int16_t gx, gy, gz;
        int offsetX = 0, offsetY = 0, offsetZ = 0;
    
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Keep hand still");
        display.println("Calibrating...");
        display.display();
        delay(2000);  
    
        for (int i = 0; i < 100; i++) {
            mpu.getRotation(&gx, &gy, &gz);
            offsetX += gx;
            offsetY += gy;
            offsetZ += gz;
            delay(10);
        }
        //Hardcoding offsets  
        gyroOffsetX = -58900 / 100;
        gyroOffsetY = 400 / 100;
        gyroOffsetZ = -1500/ 100;
        

        /*/ Calculate offsets
        gyroOffsetX = offsetX / 100;
        gyroOffsetY = offsetY / 100;
        gyroOffsetZ = offsetZ / 100;*/

        // Initialize Kalman filters with offsets
       // kalmanX.setAngle(gyroOffsetX);
        //kalmanY.setAngle(gyroOffsetY);
        
        display.clearDisplay();
        drawBatteryStatus(); 
        display.setCursor(0, 0);
        
        display.println("Calibration Done!");
        display.print("X Offset: "); display.println(gyroOffsetX);
        display.print("Y Offset: "); display.println(gyroOffsetY);
        display.print("Z Offset: "); display.println(gyroOffsetZ);
        display.display();
        delay(3000);     
        display.clearDisplay();
        Serial.print("Calibrated Offsets - X: "); Serial.print(gyroOffsetX);
        Serial.print(" Y: "); Serial.print(gyroOffsetY);
        Serial.print(" Z: "); Serial.println(gyroOffsetZ);

        
       
            
            
    }
    
void AirGlove::moveCursor() {
    if (bleMouse.isConnected()) {
        int16_t ax, ay, az;
        int16_t gx, gy, gz;
        
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        // Apply calibration
        int moveX = (gx - gyroOffsetX) / 975;
        int moveY = (gz- gyroOffsetZ) / 975;

        if (moveX != 0 || moveY != 0) {
            bleMouse.move((int8_t)moveX, (int8_t)-moveY);
        }
    }
}

void AirGlove::leftClick() {
    static bool L_flag = false;
    if (touchRead(L_click_pin) < 55 && !L_flag) {
        bleMouse.press(MOUSE_LEFT);
        L_flag = true;
    } else if (touchRead(L_click_pin) > 55 && L_flag) {
        bleMouse.release(MOUSE_LEFT);
        L_flag = false;
    }
}

void AirGlove::rightClick() {
    static bool R_flag = false;
    if (touchRead(R_click_pin) < 55 && !R_flag) {
        bleMouse.press(MOUSE_RIGHT);
        R_flag = true;
    } else if (touchRead(R_click_pin) > 55 && R_flag) {
        bleMouse.release(MOUSE_RIGHT);
        R_flag = false;
    }
}



void AirGlove::checkConnection() {
    if (!bleMouse.isConnected()) {
        digitalWrite(led_pin, HIGH);
        delay(100);
        digitalWrite(led_pin, LOW);
        delay(100);
    }
}

void AirGlove::controlLaser() {
    static bool laserState = false;   

    bool currentButtonState = (touchRead(LASER_BUTTON_PIN) < 58) ? LOW : HIGH;  

    if (currentButtonState == LOW && lastLaserButtonState == HIGH) {
        laserState = !laserState;  
        digitalWrite(LASER_PIN, laserState ? HIGH : LOW);
        Serial.println(laserState ? "Laser ON" : "Laser OFF");
        delay(200); 
    }

    lastLaserButtonState = currentButtonState; 
}


void AirGlove::show(int x, int y, int size, const char* text) {
    display.setCursor(x, y);
    display.setTextColor(WHITE);
    display.setTextSize(size);
    display.print(text);
    display.display();
}

void AirGlove::drawSmileyFace(int x, int y) {
    pinMode(MENU_SELECT_PIN, INPUT_PULLUP);  

    
    for (int i = 0; i < 360; i += 10) {
        if (digitalRead(MENU_SELECT_PIN) == LOW) {
            display.clearDisplay();
            return;  
        }

        float rad = i * PI / 180;
        int px = x + 15 * cos(rad);
        int py = y + 15 * sin(rad);
        display.drawPixel(px, py, WHITE);
        display.display();
        delay(100);
    }


    if (digitalRead(MENU_SELECT_PIN) == LOW) {
        display.clearDisplay();
        return;
    }
    display.fillCircle(x - 5, y - 5, 2, WHITE);
    display.display();
    delay(300);


    if (digitalRead(MENU_SELECT_PIN) == LOW) {
        display.clearDisplay();
        return;
    }
    display.fillCircle(x + 5, y - 5, 2, WHITE);
    display.display();
    delay(300);

    
    int smileX[] = {x - 5, x - 3, x - 1, x + 1, x + 3, x + 5};
    int smileY[] = {y + 5, y + 7, y + 8, y + 8, y + 7, y + 5};

    for (int i = 0; i < 6; i++) {
        if (digitalRead(MENU_SELECT_PIN) == LOW) {
            display.clearDisplay();
            return;
        }
        display.drawPixel(smileX[i], smileY[i], WHITE);
        display.display();
        delay(200);
    }
}

void AirGlove::displayWelcomeSequence() {

    auto waitOrSkip = [&](unsigned long duration) {
        unsigned long startTime = millis();
        while (millis() - startTime < duration) {
            if (digitalRead(MENU_SELECT_PIN) == LOW) {  
                display.clearDisplay();  
                return true;  
            }
            delay(10);  
        }
        return false;  
    };
    
    show(15, 8, 2, "WELCOME !");
    drawSmileyFace(60, 45);
    if (waitOrSkip(3000)) return;
    display.clearDisplay();

    show(3, 25, 2, "LOADING");
    for (int i = 0; i < 5; i++) {
        show(86 + (i * 6), 25, 2, ".");
        if (waitOrSkip(500)) return;
    }
    if (waitOrSkip(2000)) return;
    display.clearDisplay();

    show(10, 25, 2, "READY NOW");
    if (waitOrSkip(2000)) return;
    display.clearDisplay();

    show(10, 10, 2, "KEEP YOUR");
    show(5, 35, 2, "HAND STILL");
    if (waitOrSkip(3000)) return;
    display.clearDisplay();

    show(5, 25, 2, "SETTING UP");
    if (waitOrSkip(2000)) return;
    display.clearDisplay();

    show(5, 10, 2, "PRESENTING");
    show(52, 30, 2, "TO");
    show(46, 50, 2, "YOU");
    if (waitOrSkip(3000)) return;
    display.clearDisplay();

    show(45, 15, 2, "THE");
    show(15, 35, 2, "AIRGLOVE");
    if (waitOrSkip(3000)) return;
    display.clearDisplay();
}


void AirGlove::displayModeSelection() {
    drawBatteryStatus(); 
    show(25, 0, 1, "Select a Mode");
    show(25, 15, 1, "1. Normal Mode");
    show(25, 27, 1, "2. Gaming Mode");
    show(25, 39, 1, "3. PPT Mode");
    show(25, 51, 1, "4. Settings");
    display.display();
}

void AirGlove::handleButtonPress() {
    if (digitalRead(MENU_BUTTON_PIN) == LOW) {
        y += 12;
        if (y > SCREEN_HEIGHT - gapHeight) {
            y = 15;
        }
        digitalWrite(led_pin, HIGH);
        delay(100);
    } else {
        digitalWrite(led_pin, LOW);
    }
}

void AirGlove::handleSelection() {
    if (digitalRead(MENU_SELECT_PIN) == LOW) {
        display.clearDisplay();
        switch (y) {
            case 15:
                drawBatteryStatus();
                show(32, 5, 2, "NORMAL");
                show(40, 25, 2, "MODE");
                show(15, 45, 2, "ACTIVATED");
                break;
            case 27:
                drawBatteryStatus();
                show(30, 5, 2, "GAMING");
                show(40, 25, 2, "MODE");
                show(15, 45, 2, "ACTIVATED");
                break;
            case 39:
                show(45, 5, 2, "PPT");
                show(40, 25, 2, "MODE");
                show(15, 45, 2, "ACTIVATED");
                break;
            case 51:
                show(5, 25, 2, "SETTINGS");
                break;
        }
        display.display();
        delay(3000);
        display.clearDisplay();
        
    } 

    display.fillRect(0, 11, rectWidth, SCREEN_HEIGHT, SSD1306_WHITE);
    display.fillRect(2, y, 6, gapHeight, SSD1306_BLACK);
    display.display();
}


void AirGlove::drawBatteryStatus() {
    display.fillRect(107, 2, 14, 3, BLACK); 
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.drawRect(105, 1, 16, 6, WHITE);  // Battery outline
    display.fillRect(121, 2, 2, 3, WHITE);   // Battery terminal

    int fillWidth = map(percentage, 0, 100, 0, 14);  
    display.fillRect(107, 2, fillWidth, 3, WHITE);  

    display.display();
}
