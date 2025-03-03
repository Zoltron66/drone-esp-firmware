/*
 * File: ledManager.h
 * Project: drone_r6_fw
 * File Created: Monday, 17th February 2025 2:54:44 am
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Monday, 17th February 2025 2:54:44 am
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#pragma once

#include "DebugAndVersionControl.h"

extern "C" {
#include "driver/gpio.h"
}

// LED GPIO pins
#define LED_WS2812              GPIO_NUM_0


// Colors -------------------------------------------------------------------------------------------------------
namespace Colors {
    // RGB Color -------------------------------------------------------------------------------------------------
    class Color {
    // Init RGB Color ----------------------------------------------
    public:
        Color(uint8_t R = 0, uint8_t G = 0, uint8_t B = 0, uint8_t brightness = 100);
    
    // Colors ------------------------------------------------------
    private:
        uint8_t R;
        uint8_t G;
        uint8_t B;
        uint8_t brightness;

    // Setters and Getters -----------------------------------------
        uint8_t setOneColor(int color) const;

    public:
        void setR(int R);
        void setR(int R, int MIN, int MAX);
        void setG(int G);
        void setG(int G, int MIN, int MAX);
        void setB(int B);
        void setB(int B, int MIN, int MAX);
        void setBrightness(int brightness);
        void setBrightness(int brightness, int MIN, int MAX);

        uint8_t getRawR() const { return R; }
        uint8_t getRawG() const { return G; }
        uint8_t getRawB() const { return B; }
        uint8_t getBrightness() const { return brightness; }

        uint8_t getR() const { return setOneColor(float(brightness) / 100 * R); }
        uint8_t getG() const { return setOneColor(float(brightness) / 100 * G); }
        uint8_t getB() const { return setOneColor(float(brightness) / 100 * B); }

    // Compare RGB Colors ------------------------------------------
    public:
        bool operator==(const Color &color) const;
    };

    // Default Gadget RGB Colors
    static const Color Orange(110U, 20U, 0U);   // 
    static const Color Red(250U, 8U, 0U);       // OK
    static const Color Blue(0U, 50U, 255U);     //

    // Reflector RGB Color
    static const Color White(255U, 255U, 255U); // Obviously OK

    // Extra Gadget RGB Colors
    static const Color Pink(250U, 2U, 20U);     // OK

    // Utility RGB Colors
    static const Color Wifi(10U, 100U, 250U);   // OK
    static const Color Error(255U, 0U, 0U);     // OK

    // OFF RGB Color
    static const Color Off(0U, 0U, 0U, 0U);         // Obviously OK
}


// LED ----------------------------------------------------------------------------------------------------------
class Led {
// Init LED ---------------------------------------------------
public:
    Led() {};

// LED stages -------------------------------------------------
public:
    Colors::Color currentColorStage;

private:
    Colors::Color targetColorStage;

public:    
    bool moveTowardsToTargetStage(int speed, int MINB = 0, int MAXB = 100, int MINC = 0, int MAXC = 255);

// LED controls -----------------------------------------------
public:
    void setColor(Colors::Color color) { currentColorStage = color; targetColorStage = color; }

    void setCurrentColor(Colors::Color color) { currentColorStage = color; }
    void setCurrentColorBrightness(float brightness) { currentColorStage.setBrightness(brightness); }

    void setTargetColor(Colors::Color color) { targetColorStage = color; }
    void setTargetColorBrightness(float brightness) { targetColorStage.setBrightness(brightness); }

    void setOff() { currentColorStage = Colors::Off; targetColorStage = Colors::Off; }
};

// Animation Types -----------------------------------------------------------------------------------------------
enum AnimationType {
    NONE,

    // Default Gadget Animations
    IDLE,
    IDLE_DEBUG,

    // Extra Gadget Animations
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_DISCONNECTED // TODO: When wifi disconnects request a /dis connection from the server, and set the led to red color
};


// LED Manager --------------------------------------------------------------------------------------------------
class LedManager {
// Init LED manager ---------------------------------------------------
private:
    LedManager();

// LED manager controls ----------------------------------------------
private:
    Colors::Color debugColor, currentColor;
    
public:
    void setDebugColor(Colors::Color color) { debugColor = color; }

    void setColor(Colors::Color color) { currentColor = color; }

    bool moveAllLedTowardsToTargetColor(int MINB = 0, int MAXB = 100, int MINC = 0, int MAXC = 255);

    void setAllOff();

// Animation controls -------------------------------------------------
private:
    AnimationType currentAnimation, lastAnimation;
    uint8_t animationSpeed;

public:
    void setAnimation(AnimationType animation) { currentAnimation = animation; }
    void resetAnimation() { currentAnimation = AnimationType::NONE; lastAnimation = AnimationType::NONE; }
    void setAnimationSpeed(uint8_t speed) { animationSpeed = speed; }

    AnimationType getCurrentAnimation() const { return currentAnimation; }

// Animation sequences -----------------------------------------------
private:
    uint8_t animationStage;

    void resetAnimationStageIfChanged(AnimationType animation, uint8_t speed = 1);

public:
    void playNone();

    void playIdleAnimation();

#ifdef VERSION_1_OR_LATER
    void playIdleDebugAnimation();
#endif

    void playWifiConnectingAnimation();

    void playWifiConnectedAnimation();

    void playWifiDisconnectedAnimation();

// LED array controls -------------------------------------------------
private:
    Led LED[6];

public:
    void transmitWaveformToLedArray();

    void startLedArrayControls();

// Deinit LED manager -------------------------------------------------
public:
     ~LedManager();

// Singleton ----------------------------------------------------------
private:
    static LedManager* instance;

public:
    LedManager(const LedManager& ledManager) = delete;

    LedManager& operator=(const LedManager& ledManager) = delete;

    static void init();

    static LedManager* getInstance() { return instance; }

    static void deinit();
};