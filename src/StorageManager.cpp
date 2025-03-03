/*
 * File: StorageManager.cpp
 * Project: drone_r6_fw
 * File Created: Thursday, 27th February 2025 8:50:26 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Thursday, 27th February 2025 8:50:27 pm
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#include "StorageManager.h"

// Storage manager ---------------------------------------------------------------------
// Init storage ---------------------------------------------------------
StorageManager::StorageManager() {
    DEBUG_INIT_START("Storage manager");
    // Init datas
    WiFiSSID = "";
    WiFiPassword = "";
    ColorNumber = 0;

    // Init NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
        ESP_ERROR_CHECK(err);
    }
    
    // Open NVS
    nvsHandle = nvs::open_nvs_handle("storage_manager", NVS_READWRITE, &err);
    ESP_ERROR_CHECK(err);

    DEBUG_INIT_END("Storage manager");
}

// Storage management ---------------------------------------------------
// Get data from storage ------------------------------------------------
void StorageManager::getWifiSSIDDataFromStorage() {
    size_t dataSize;
    esp_err_t err = nvsHandle->get_item_size(nvs::ItemType::SZ, "WIFI_SSID", dataSize);
    char *WiFiSSIDData = new char[dataSize];
    err = nvsHandle->get_string("WIFI_SSID", WiFiSSIDData, dataSize);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        WiFiSSID = WIFI_SSID;
        return;
    }
    WiFiSSID = WiFiSSIDData;
    delete[] WiFiSSIDData;
}

void StorageManager::getWifiPasswordDataFromStorage() {
    size_t dataSize;
    esp_err_t err = nvsHandle->get_item_size(nvs::ItemType::SZ, "WIFI_PASSWORD", dataSize);
    char *WiFiPasswordData = new char[dataSize];
    err = nvsHandle->get_string("WIFI_PASSWORD", WiFiPasswordData, dataSize);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        WiFiPassword = WIFI_PASSWORD;
        return;
    }
    WiFiPassword = WiFiPasswordData;
    delete[] WiFiPasswordData;
}

void StorageManager::getColorDataFromStorage() {
    esp_err_t err = nvsHandle->get_item("COLOR_NUMBER", ColorNumber);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ColorNumber = 0;
        return;
    }
}

// Commit data to storage ------------------------------------------------
void StorageManager::commitWifiSSIDDataToStorage() {
    esp_err_t err = nvsHandle->set_string("WIFI_SSID", WiFiSSID.c_str());
    ESP_ERROR_CHECK(err);
}

void StorageManager::commitWifiPasswordDataToStorage() {
    esp_err_t err = nvsHandle->set_string("WIFI_PASSWORD", WiFiPassword.c_str());
    ESP_ERROR_CHECK(err);
}

void StorageManager::commitColorDataToStorage() {
    esp_err_t err = nvsHandle->set_item("COLOR_NUMBER", ColorNumber);
    ESP_ERROR_CHECK(err);
}

// Set data ------------------------------------------------------------
void StorageManager::setWiFiSSID(const std::string ssid) {
    if (ssid.length() > 30) {
        return;
    }
    WiFiSSID = ssid;
    commitWifiSSIDDataToStorage();
}

void StorageManager::setWiFiPassword(const std::string password) {
    if (password.length() > 30) {
        return;
    }
    WiFiPassword = password;
    commitWifiPasswordDataToStorage();
}

void StorageManager::setColorNumber(int8_t colorNumber) {
    ColorNumber = colorNumber;
    commitColorDataToStorage();
}

// Get all data from storage --------------------------------------------
void StorageManager::getAllDataFromStorage() {
    getWifiSSIDDataFromStorage();
    getWifiPasswordDataFromStorage();
    getColorDataFromStorage();
}

// Reset memory to default ----------------------------------------------
void StorageManager::resetMemoryToDefault() {
    DEBUG_PRINT("Resetting memory to default...");
    init();
    StorageManager* storageManager = StorageManager::getInstance();
    if (!storageManager) {
        return;
    }
    storageManager->setWiFiSSID(WIFI_SSID);
    storageManager->setWiFiPassword(WIFI_PASSWORD);
    storageManager->setColorNumber(0);
    deinit();
    DEBUG_PRINT("Memory resetted to default");
}

// Deinit storage -------------------------------------------------------
StorageManager::~StorageManager() {
    DEBUG_DEINIT_START("Storage manager");
    nvsHandle.reset();
    DEBUG_DEINIT_END("Storage manager");
}

// Singleton ------------------------------------------------------------
StorageManager* StorageManager::instance = nullptr;

void StorageManager::init() {
    if (instance == nullptr) {
        instance = new StorageManager();
    }
}

void StorageManager::deinit() {
    if (instance) {
        delete instance;
        instance = nullptr;
        return;
    }
    DEBUG_DEINIT_NO_NEED("Storage manager");
}