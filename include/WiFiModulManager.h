/*
 * File: networkManager.h
 * Project: drone_r6_fw
 * File Created: Wednesday, 19th February 2025 5:44:01 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Wednesday, 19th February 2025 5:44:02 pm
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#pragma once

#include "DebugAndVersionControl.h"

// C++
#include <iostream>

// C
extern "C" {
#include "esp_err.h"
#include "esp_wifi.h"

#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}


#ifdef VERSION_BETA_OR_LATER
enum SignalStrenght {
    EXCELENT,  // -30 dBm to -50 dBm
    GOOD,      // -50 dBm to -60 dBm
    FAIR,      // -60 dBm to -67 dBm
    WEAK,      // -67 dBm to -70 dBm
    VERY_WEAK, // -70 dBm to -80 dBm
    NOSIGNAL   // No Signal
};
#endif

enum NetworkStatus {
    DISCONNECTED,
    TRYING_TO_CONNECT,
    TRYING_TO_RECONNECT,
    CONNECTED
};

class WiFiModulManager {
// Init wifi ------------------------------------------------------------
private:
    WiFiModulManager();

// Connect to wifi ------------------------------------------------------
private:
    std::string ssid;
    std::string password;
    NetworkStatus networkStatus;
#ifdef VERSION_BETA_OR_LATER
    SignalStrenght signalStrenght;
#endif

public:
    void setSSID(const char* ssid) { this->ssid = ssid; }
    void setPassword(const char* password) { this->password = password; }
    void setNetworkStatus(NetworkStatus networkStatus) { this->networkStatus = networkStatus; }
    NetworkStatus getNetworkStatus() { return networkStatus; }
#ifdef VERSION_BETA_OR_LATER
    //void refreshSignalStrenght();

    SignalStrenght getSignalStrenght() const { return signalStrenght; }
#endif

    void connectToWiFi();

// Get gateway infos -----------------------------------------------------
private:
    std::string gatewayIP;
    std::string subnetMask;
    bool isStaticIpSet;

public:
    std::string getGatewayIP() { return gatewayIP; }
    std::string getSubnetMask() { return subnetMask; }
    void geatherGatewayInfos(char* gatewayIP, char* subnetMask) { this->gatewayIP = gatewayIP; this->subnetMask = subnetMask; }
    bool isGatewayInfosEmpty() { return gatewayIP.empty() || subnetMask.empty(); }

// Reconnect to wifi -----------------------------------------------------
public:
    void reconnectToWiFi();

// Disconnect from wifi -------------------------------------------------
public:
    void disconnectFromWiFi();

// WiFi Controls --------------------------------------------------------
public:
    void startWiFiControls();

// Deinit wifi ----------------------------------------------------------
public:
    ~WiFiModulManager();

// Singleton ------------------------------------------------------------
private:
    static WiFiModulManager* instance;

public:
    WiFiModulManager(const WiFiModulManager& wifiModulManager) = delete;

    WiFiModulManager& operator=(const WiFiModulManager& wifiModulManager) = delete;

    //static void init();

    static WiFiModulManager* getInstance();

    static void deinit() { delete instance; }
};

// FIXME: Creat the singleton init and deinit method for the WiFiModulManager, and rewrite the getInstance method to not create a new instance



