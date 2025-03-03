/*
 * File: StorageManager.h
 * Project: drone_r6_fw
 * File Created: Monday, 17th February 2025 7:02:09 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Monday, 17th February 2025 7:02:09 pm
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#pragma once

#include "DebugAndVersionControl.h"
#include "AuthAndPasswords.h" // Comment out if not nexist

// C++
#include <iostream>
#include "nvs_handle.hpp"
#include "nvs_flash.h"
#include "nvs.h"

#ifndef AUTH_AND_PASSWORDS
#define WIFI_SSID "Your wifi SSID"
#define WIFI_PASSWORD "Your wifi password"
#endif



// init storage manager
// get data from storage
// check if data storage is full or other problem occured
// if yes
//      delete data from storage
//      save gained data to storage
// if not
//      do nothing


// Plus save to storage functions

// Storage manager ---------------------------------------------------------------------
class StorageManager {
// Init storage ---------------------------------------------------------
private:
    StorageManager();

// Datas ----------------------------------------------------------------
private:
    // Storage handle
    std::unique_ptr<nvs::NVSHandle> nvsHandle;

    // WiFi data
    std::string WiFiSSID;
    std::string WiFiPassword;
    // Color data
    int8_t ColorNumber;

// Storage management ---------------------------------------------------
private:
    void getWifiSSIDDataFromStorage();
    void getWifiPasswordDataFromStorage();
    void getColorDataFromStorage();

    void commitWifiSSIDDataToStorage();
    void commitWifiPasswordDataToStorage();
    void commitColorDataToStorage();

public:
    void setWiFiSSID(const std::string ssid);
    void setWiFiPassword(const std::string password);
    void setColorNumber(int8_t colorNumber);

    std::string getWiFiSSID() const { return WiFiSSID; }
    std::string getWiFiPassword() const { return WiFiPassword; }
    int8_t getColorNumber() const { return ColorNumber; }

    void getAllDataFromStorage();

    static void resetMemoryToDefault();

// Deinit storage -------------------------------------------------------
public:
    ~StorageManager();

// Singleton ------------------------------------------------------------
private:
    static StorageManager* instance;

public:
    StorageManager(const StorageManager& storageManager) = delete;

    StorageManager& operator=(const StorageManager& storageManager) = delete;

    static void init();

    static StorageManager* getInstance() { return instance; }

    static void deinit();
};
