/*
 * File: LedManagerUnitTest.cpp
 * Project: drone_r6_fw
 * File Created: Wednesday, 26th February 2025 11:18:55 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Wednesday, 26th February 2025 11:18:55 pm
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#include "UnitTests.h"

#ifdef UNIT_TESTS

#define LED_COLOR_TEST
#define LED_ANIMATION_TEST

/**
 * @brief Unit test for LED Manager
 * 
 * @param isLoop 
 * 
 * @note Test cases:
 * init,
 * startLedArrayControls,
 * setColor (with all color),
 * setAnimation (with all animations),
 * deinit
 */
void UnitTests::LedManagerUnitTest(bool isLoop) {
    TEST_START("LED Manager");
    do {
        UNIT_PRINT("Init LED manager...");
        LedManager::init();

        UNIT_PRINT("Getting LED manager instance...");
        LedManager* ledManager = LedManager::getInstance();
        if (!ledManager) {
            UNIT_PRINT("LED manager instance is nullptr...");
            TEST_END_FAILED("LED Manager");
            return;
        }

        UNIT_PRINT("Starting LED array controls...");
        ledManager->startLedArrayControls();

#ifdef LED_COLOR_TEST
        UNIT_PRINT("Setting color to ORANGE and playing IDLE animation...");
        ledManager->setColor(Colors::Orange);
        ledManager->setAnimation(AnimationType::IDLE);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ledManager->resetAnimation();

        UNIT_PRINT("Setting color to RED and playing IDLE animation...");
        ledManager->setColor(Colors::Red);
        ledManager->setAnimation(AnimationType::IDLE);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ledManager->resetAnimation();

        UNIT_PRINT("Setting color to BLUE and playing IDLE animation...");
        ledManager->setColor(Colors::Blue);
        ledManager->setAnimation(AnimationType::IDLE);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ledManager->resetAnimation();

        UNIT_PRINT("Setting color to WHITE and playing IDLE animation...");
        ledManager->setColor(Colors::White);
        ledManager->setAnimation(AnimationType::IDLE);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ledManager->resetAnimation();

        UNIT_PRINT("Setting color to PINK and playing IDLE animation...");
        ledManager->setColor(Colors::Pink);
        ledManager->setAnimation(AnimationType::IDLE);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ledManager->resetAnimation();

        UNIT_PRINT("Setting color to WIFI and playing IDLE animation...");
        ledManager->setColor(Colors::Wifi);
        ledManager->setAnimation(AnimationType::IDLE);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ledManager->resetAnimation();

        UNIT_PRINT("Setting color to ERROR and playing IDLE animation...");
        ledManager->setColor(Colors::Error);
        ledManager->setAnimation(AnimationType::IDLE);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ledManager->resetAnimation();

        UNIT_PRINT("Setting color to OFF and playing IDLE animation...");
        ledManager->setColor(Colors::Off);
        ledManager->setAnimation(AnimationType::IDLE);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ledManager->resetAnimation();

#endif
#ifdef LED_ANIMATION_TEST
        UNIT_PRINT("Playing IDLE animation (with Red color)...");
        ledManager->setColor(Colors::Red);
        ledManager->setAnimation(AnimationType::IDLE);
        vTaskDelay(10000 / portTICK_PERIOD_MS);

#ifdef VERSION_1_OR_LATER
        UNIT_PRINT("Playing IDLE_DEBUG animation (with Blue color)...");
        ledManager->setColor(Colors::Blue);
        ledManager->setAnimation(AnimationType::IDLE_DEBUG);
        vTaskDelay(20000 / portTICK_PERIOD_MS);
#endif

        UNIT_PRINT("Playing WIFI_CONNECTING animation..."); // Wifi Color
        ledManager->setAnimation(AnimationType::WIFI_CONNECTING);
        vTaskDelay(10000 / portTICK_PERIOD_MS);

        UNIT_PRINT("Playing WIFI_CONNECTED animation..."); // Wifi Color -> Green Color
        ledManager->setColor(Colors::Pink);
        ledManager->setAnimation(AnimationType::WIFI_CONNECTED);
        vTaskDelay(8000 / portTICK_PERIOD_MS);
        // This is a one time animation, no need to reset when it's normaly finished
        // After this animation, the animation will be IDLE
        
        UNIT_PRINT("Playing WIFI_DISCONNECTED animation..."); // Error Color
        ledManager->setAnimation(AnimationType::WIFI_DISCONNECTED);
        vTaskDelay(8000 / portTICK_PERIOD_MS);
        // This is a one time animation, no need to reset when it's normaly finished
        // After this animation, the animation will be NONE
#endif

        UNIT_PRINT("Deinit LED manager...");
        LedManager::deinit();

        TEST_END_PASSED("LED Manager");
    } while (isLoop);
}

#endif


// DONE: LED Manager in verison 0.1.0 (ALPHA)
