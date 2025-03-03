/*
 * File: serverManager.h
 * Project: drone_r6_fw
 * File Created: Sunday, 16th February 2025 3:54:12 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Monday, 17th February 2025 2:47:08 am
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#pragma once

#include "DebugAndVersionControl.h"

extern "C" {
#include "esp_http_server.h"
}

// Server Manager ----------------------------------------------------------------
class ServerManager {
// Init server manager ---------------------------------------------------
private:
    ServerManager();

// Command Server --------------------------------------------------------
private:
    // httpd_uri_t indexUri;
    httpd_uri_t connectionUri;
    httpd_uri_t disconnectionUri;
    httpd_uri_t moveUri;
    httpd_uri_t getSettingsUri;
    httpd_uri_t setWiFiUri;
    httpd_uri_t setLedUri;
    httpd_uri_t setRoomPlantModeUri;

// Video Server ----------------------------------------------------------
private:
    httpd_uri_t streamUri;

// Servers ---------------------------------------------------------------
public:
    void startServers();

// Deinit server manager -------------------------------------------------
public:
    ~ServerManager();   

// Singleton ------------------------------------------------------------- 
private:
    static ServerManager* instance;

public:
    ServerManager(const ServerManager& serverManager) = delete;

    ServerManager& operator=(const ServerManager& serverManager) = delete;

    static ServerManager* getInstance();

    static void deinit() { delete instance; }
};

// DONE: ServerManager.h VERSION_ALPHA