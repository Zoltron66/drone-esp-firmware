/*
 * File: StorageManagerUnitTest.cpp
 * Project: drone_r6_fw
 * File Created: Thursday, 27th February 2025 9:52:20 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Thursday, 27th February 2025 9:52:20 pm
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#include "UnitTests.h"

#ifdef UNIT_TESTS

void UnitTests::StorageManagerUnitTest(bool isLoop) {
    TEST_START("Storage Manager");
    do {
        UNIT_PRINT("Init Storage manager...");
        StorageManager::init();
        
        UNIT_PRINT("Get Storage manager instance...");
        StorageManager* storageManager = StorageManager::getInstance();
        if (!storageManager) {
            UNIT_PRINT("Storage manager instance is nullptr...");
            TEST_END_FAILED("Storage Manager");
            return;
        }
        storageManager->getAllDataFromStorage();
        if (storageManager->getColorNumber() != 5) {
            UNIT_PRINT("Getting all data from storage (Nothing should be stored yet)...");
        
            UNIT_PRINT("WiFi SSID: %s", storageManager->getWiFiSSID().c_str());
            UNIT_PRINT("WiFi Password: %s", storageManager->getWiFiPassword().c_str());
            UNIT_PRINT("Color Number: %d", storageManager->getColorNumber());

            UNIT_PRINT("Setting WiFi SSID to 'Test ssid 01'...");
            storageManager->setWiFiSSID("Test ssid 01");

            UNIT_PRINT("Setting WiFi Password to 'Test password 01'...");
            storageManager->setWiFiPassword("Test password 01");

            UNIT_PRINT("Setting Color Number to 5...");
            storageManager->setColorNumber(5);

            UNIT_PRINT("Restarting ESP32 in 5 seconds...");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            TEST_END_PASSED("If the ESP32 restarts and the data is stored, this text will not be printed again.");

            UNIT_PRINT("Deinit Storage manager...");
            StorageManager::deinit();

            UNIT_PRINT("Restarting ESP32...");
            esp_restart();
        } else {
            UNIT_PRINT("Getting all data from storage (After restart)...");

            UNIT_PRINT("WiFi SSID: %s", storageManager->getWiFiSSID().c_str());
            UNIT_PRINT("WiFi Password: %s", storageManager->getWiFiPassword().c_str());
            UNIT_PRINT("Color Number: %d", storageManager->getColorNumber());

            UNIT_PRINT("Deinit Storage manager...");
            StorageManager::deinit();
            TEST_END_PASSED("Storage Manager");
        }
    } while (isLoop);
}

#endif

// DONE: Storage Manager in verison 0.1.0 (ALPHA)