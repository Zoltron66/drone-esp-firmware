/*
 * File: ServerManager.cpp
 * Project: drone_r6_fw
 * File Created: Thursday, 20th February 2025 3:44:41 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Thursday, 20th February 2025 3:44:41 pm
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#include "ServerManager.h"
#include "MotorManager.h"
#include "LedManager.h"
#include <iostream>

extern "C" {
#include "esp_camera.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_timer.h"
}

// Server Manager -------------------------------------------------------------------
// Command Server -----------------------------------------------------------
static esp_err_t connectionHandler(httpd_req_t *req) {
    LedManager::getInstance()->setAnimation(AnimationType::IDLE);
    return httpd_resp_send(req, "OK", 2);
}

static esp_err_t disconnectionHandler(httpd_req_t *req) {
    LedManager::getInstance()->setAnimation(AnimationType::NONE);
    return httpd_resp_send(req, nullptr, 0);
}

static esp_err_t moveHandler(httpd_req_t *req) {
    char*  buf;
    size_t bufferLength;
    char axisValueInChar[10] = {0,};
    int16_t XAxisValue = 0;
    int16_t YAxisValue = 0;
    int16_t LDirectionValue = 0;
    int16_t RDirectionValue = 0;

    bufferLength = httpd_req_get_url_query_len(req) + 1;
    if (bufferLength > 1) {
        buf = (char*)malloc(bufferLength);
        if(!buf){
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        // Get the query parameter data
        if (httpd_req_get_url_query_str(req, buf, bufferLength) != ESP_OK) {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        // Get the value of the query parameter
        if (httpd_query_key_value(buf, "X", axisValueInChar, sizeof(axisValueInChar)) != ESP_OK) {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        XAxisValue = atoi(axisValueInChar);

        if (httpd_query_key_value(buf, "Y", axisValueInChar, sizeof(axisValueInChar)) != ESP_OK) {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        YAxisValue = atoi(axisValueInChar);

        if (httpd_query_key_value(buf, "L", axisValueInChar, sizeof(axisValueInChar)) != ESP_OK) {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        LDirectionValue = atoi(axisValueInChar);

        if (httpd_query_key_value(buf, "R", axisValueInChar, sizeof(axisValueInChar)) != ESP_OK) {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        RDirectionValue = atoi(axisValueInChar);

        free(buf);
    } else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    MotorManager* motorManager = MotorManager::getInstance();
    DEBUG_PRINT("X: %d, Y: %d, L: %d, R: %d", XAxisValue, YAxisValue, LDirectionValue, RDirectionValue);
    motorManager->setControlData(XAxisValue, YAxisValue, LDirectionValue, RDirectionValue);
    return httpd_resp_send(req, nullptr, 0);
}

static esp_err_t getSettingsHandler(httpd_req_t *req) {
    // JSON format
    // {
    //      "ssid": "ssid",
    //      "password": "password",
    //      "color": 2
    // }

    // TODO: Implement the getSettingsHandler (JSON)
    // httpd_resp_set_type(req, "application/json");
    // std::string ssidS = WiFiModulManager::getInstance()->getSSID();
    // std::string passwordS = WiFiModulManager::getInstance()->getPassword();
    // std::string colorS = LedManager::getInstance()->getLedColor();
    // std::string settings = "{\"ssid\": \"" + ssidS + "\", \"password\": \"" + passwordS + "\", \"color\": " + colorS + "}";
    // return httpd_resp_send(req, settings.c_str(), settings.length());
    return httpd_resp_send(req, nullptr, 0);
}

static esp_err_t setWiFiHandler(httpd_req_t *req) {
    // Something like the moveHandler
    // TODO: Implement the setWiFiHandler
    // WiFiModulManager* wifiModulManager = WiFiModulManager::getInstance();
    // wifiModulManager->setNextSSID("ssid");
    // wifiModulManager->setNextPassword("password");
    return httpd_resp_send(req, nullptr, 0);
}

static esp_err_t setLedHandler(httpd_req_t *req) {
    // Something like the moveHandler
    // TODO: Implement the setLedHandler
    // LedManager* ledManager = LedManager::getInstance();
    // ledManager->setLedColor(2);
    return httpd_resp_send(req, nullptr, 0);
}

static esp_err_t setRoomPlantHandler(httpd_req_t *req) {
    // TODO: Implement the setRoomPlantHandler
    // ModeManager::getInstance()->setMode(ROOM_PLANT);
    return httpd_resp_send(req, nullptr, 0);
}

// Video Server -------------------------------------------------------------
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static esp_err_t streamHandler(httpd_req_t *req) {
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t jpgBufferLength = 0;
    uint8_t * jpgBuffer = nullptr;
    char * partitionBuffer[64];
    // static int64_t lastFrame = 0;

    // if(!lastFrame) { lastFrame = esp_timer_get_time(); }
    res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if (res != ESP_OK) { return res; }

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            DEBUG_PRINT("Camera capture failed");
            res = ESP_FAIL;
            break;
        }
        if (fb->format != PIXFORMAT_JPEG) {
            if (!frame2jpg(fb, 80, &jpgBuffer, &jpgBufferLength)) {
                DEBUG_PRINT("JPEG compression failed");
                esp_camera_fb_return(fb);
                res = ESP_FAIL;
            }
        } else {
            jpgBufferLength = fb->len;
            jpgBuffer = fb->buf;
        }
        if (res == ESP_OK) { res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY)); }
        if (res == ESP_OK) {
            size_t hlen = snprintf((char *)partitionBuffer, 64, STREAM_PART, jpgBufferLength);
            res = httpd_resp_send_chunk(req, (const char *)partitionBuffer, hlen);
        }
        if (res == ESP_OK) { res = httpd_resp_send_chunk(req, (const char *)jpgBuffer, jpgBufferLength); }
        if (fb->format != PIXFORMAT_JPEG) { free(jpgBuffer); }
        
        esp_camera_fb_return(fb);
        if (res != ESP_OK) { break; }   
    }
    return res;
}

ServerManager::ServerManager() {
    DEBUG_PRINT("--- Init Servers called");
    connectionUri = {
        .uri = "/con",
        .method = HTTP_GET,
        .handler = connectionHandler,
        .user_ctx = nullptr
    };

    disconnectionUri = {
        .uri = "/dis",
        .method = HTTP_GET,
        .handler = disconnectionHandler,
        .user_ctx = nullptr
    };

    moveUri = {
        .uri = "/mov",
        .method = HTTP_GET,
        .handler = moveHandler,
        .user_ctx = nullptr
    };

    getSettingsUri = {
        .uri = "/gst",
        .method = HTTP_GET,
        .handler = getSettingsHandler,
        .user_ctx = nullptr
    };

    setWiFiUri = {
        .uri = "/wif",
        .method = HTTP_GET,
        .handler = setWiFiHandler,
        .user_ctx = nullptr
    };

    setLedUri = {
        .uri = "/led",
        .method = HTTP_GET,
        .handler = setLedHandler,
        .user_ctx = nullptr
    };

    setRoomPlantModeUri = {
        .uri = "/rpl",
        .method = HTTP_GET,
        .handler = setRoomPlantHandler,
        .user_ctx = nullptr
    };

    // Video Server -------------------------------------------------------------
    streamUri = {
        .uri = "/str",
        .method = HTTP_GET,
        .handler = streamHandler,
        .user_ctx = nullptr
    };
    DEBUG_PRINT("Servers inited ---");
}

// Servers ------------------------------------------------------------------
static httpd_handle_t commandServer = nullptr;
static httpd_handle_t videoServer = nullptr;

void ServerManager::startServers() {
    DEBUG_PRINT("--- Starting servers");
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    if (httpd_start(&commandServer, &config) == ESP_OK) {
        httpd_register_uri_handler(commandServer, &connectionUri);
        httpd_register_uri_handler(commandServer, &disconnectionUri);
        httpd_register_uri_handler(commandServer, &moveUri);
        httpd_register_uri_handler(commandServer, &getSettingsUri);
        httpd_register_uri_handler(commandServer, &setWiFiUri);
        httpd_register_uri_handler(commandServer, &setLedUri);
        httpd_register_uri_handler(commandServer, &setRoomPlantModeUri);
    } else { DEBUG_PRINT("Failed to start command server"); }
    config.server_port = 81;
    config.ctrl_port += 1;
    if (httpd_start(&videoServer, &config) == ESP_OK) {
        esp_err_t vari = httpd_register_uri_handler(videoServer, &streamUri);
        if (vari != ESP_OK) { DEBUG_PRINT("Failed to register URI handler"); }
    } else { DEBUG_PRINT("Failed to start video server"); }
    DEBUG_PRINT("Servers started ---");
}

// Deinit server manager ----------------------------------------------------
ServerManager::~ServerManager() {
    DEBUG_PRINT("--- Deinit Servers called");
    if (commandServer) { httpd_stop(commandServer); }
    if (videoServer) { httpd_stop(videoServer); }
    delete instance;
    DEBUG_PRINT("Servers deinited ---");
}

// Singleton ----------------------------------------------------------------
ServerManager* ServerManager::instance = nullptr;

ServerManager* ServerManager::getInstance() {
    if (instance == nullptr) {
        instance = new ServerManager();
    }
    return instance;
}