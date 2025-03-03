/*
 * File: WiFiModulManagerUnitTest.cpp
 * Project: drone_r6_fw
 * File Created: Thursday, 27th February 2025 5:35:52 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Thursday, 27th February 2025 5:35:52 pm
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#include "UnitTests.h"

#ifdef UNIT_TESTS
#include "AuthAndPasswords.h" // Comment if it not exists

/*
    With this configuration we can use the wifi, without the need to initialize 
    the Storage Manager (which stores the wifi credentials).
*/
#ifndef AUTH_AND_PASSWORDS
#define WIFI_SSID "Your wifi SSID"          // SSID (Name) of your wifi
#define WIFI_PASSWORD "Your wifi password"  // Password of your wifi
#endif


/**
 * @brief Unit test for WiFi Modul Manager
 * 
 * @param isLoop
 * 
 * @note Test cases:
 * init,
 * setSSID,
 * setPassword,
 * startWiFiControls
 * setNetworkStatus (with all status),
 * disconnectFromWiFi,
 * deinit
 */
void UnitTests::WiFiModulManagerUnitTest(bool isLoop) {
    TEST_START("WiFi Modul Manager");
    do {
        UNIT_PRINT("Init Led manager...");
        LedManager* ledManager = LedManager::getInstance();
        
        UNIT_PRINT("Starting LED array controls...");
        ledManager->startLedArrayControls();

        UNIT_PRINT("Init WiFi modul manager...");
        WiFiModulManager* wifiModulManager = WiFiModulManager::getInstance();

        UNIT_PRINT("Setting SSID and password...");
        wifiModulManager->setSSID(WIFI_SSID);
        wifiModulManager->setPassword(WIFI_PASSWORD);

        UNIT_PRINT("Starting WiFi controls...");
        wifiModulManager->startWiFiControls();

        UNIT_PRINT("Waiting for the wifi to connect and do some stuff...");
        vTaskDelay(10000 / portTICK_PERIOD_MS);

        UNIT_PRINT("Manual disconnecting from the wifi in 5 seconds...");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        wifiModulManager->disconnectFromWiFi();

        UNIT_PRINT("Asking the wifi controls to reconnect in 4 seconds...");
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        wifiModulManager->setNetworkStatus(TRYING_TO_CONNECT);

        UNIT_PRINT("Waiting for the wifi to connect and do some stuff...");
        vTaskDelay(8000 / portTICK_PERIOD_MS);
        if (wifiModulManager->getNetworkStatus() == CONNECTED) {
            UNIT_PRINT("WiFi connected");

            UNIT_PRINT("Manual disconnecting from the wifi in 5 seconds...");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            wifiModulManager->disconnectFromWiFi();

            // UNIT_PRINT("Deinit WiFi modul manager...");
            // delete wifiModulManager;

            TEST_END_PASSED("WiFi Modul Manager");
        } else {
            // UNIT_PRINT("Deinit WiFi modul manager...");
            // WiFiModulManager::deinit();




            TEST_END_FAILED("WiFi Modul Manager");
        }
    } while (isLoop);
}
#endif