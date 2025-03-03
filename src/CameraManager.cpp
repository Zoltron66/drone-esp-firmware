/*
 * File: CameraManager.cpp
 * Project: drone_r6_fw
 * File Created: Thursday, 20th February 2025 9:42:01 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Thursday, 20th February 2025 9:42:01 pm
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#include "CameraManager.h"


// Init camera ----------------------------------------------------------
CameraManager::CameraManager() {
    DEBUG_INIT_START("Camera");
    cameraConfig = {
        .pin_pwdn = CAMERA_PWDN,
        .pin_reset = CAMERA_RESET,
        .pin_xclk = CAMERA_XCLK,
        .pin_sscb_sda = CAMERA_SIOD,
        .pin_sscb_scl = CAMERA_SIOC,
        
        .pin_d7 = CAMERA_Y9,
        .pin_d6 = CAMERA_Y8,
        .pin_d5 = CAMERA_Y7,
        .pin_d4 = CAMERA_Y6,
        .pin_d3 = CAMERA_Y5,
        .pin_d2 = CAMERA_Y4,
        .pin_d1 = CAMERA_Y3,
        .pin_d0 = CAMERA_Y2,

        .pin_vsync = CAMERA_VSYNC,
        .pin_href = CAMERA_HREF,
        .pin_pclk = CAMERA_PCLK,
        
        .xclk_freq_hz = CAMERA_XCLK_FREQ_HZ,
        .ledc_timer = CAMERA_TIMER,
        .ledc_channel = CAMERA_CHANNEL,
        
        .pixel_format = CAMERA_PIXEL_FORMAT,

        .frame_size = CAMERA_FRAMESIZE,
        .jpeg_quality = CAMERA_JPEG_QUALITY,
        .fb_count = CAMERA_FB_COUNT,
        .fb_location = CAMERA_FB_LOCATION,

        .grab_mode = CAMERA_GRAB_MODE,
        
        .sccb_i2c_port = -1
    };
    esp_err_t err = esp_camera_init(&cameraConfig);
    if (err != ESP_OK) {
        DEBUG_PRINT("Camera init failed with error: %d", err);
        ESP_ERROR_CHECK(err);
        return;
    }
    DEBUG_INIT_END("Camera");
}

// Deinit camera --------------------------------------------------------
CameraManager::~CameraManager() {
    DEBUG_DEINIT_START("Camera");
    esp_camera_deinit();
    DEBUG_DEINIT_END("Camera");
}

// Singleton ------------------------------------------------------------
CameraManager* CameraManager::instance = nullptr;

void CameraManager::init() {
    if (instance == nullptr) {
        instance = new CameraManager();
        return;
    }
    DEBUG_INIT_NO_NEED("Camera manager");
}

void CameraManager::deinit() {
    if (instance) {
        delete instance;
        instance = nullptr;
        return;
    }
    DEBUG_DEINIT_NO_NEED("Camera manager");
}


// DONE: CameraManager.cpp VERSION_ALPHA