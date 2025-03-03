/*
 * File: cameraManager.h
 * Project: drone_r6_fw
 * File Created: Monday, 17th February 2025 2:49:35 am
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Monday, 17th February 2025 2:49:35 am
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#pragma once

#include "DebugAndVersionControl.h"

// C
extern "C" {
#include "esp_camera.h"
}

// Camera GPIO pins
#define CAMERA_PWDN             GPIO_NUM_32
#define CAMERA_RESET            GPIO_NUM_NC
#define CAMERA_XCLK             GPIO_NUM_0
#define CAMERA_SIOD             GPIO_NUM_26
#define CAMERA_SIOC             GPIO_NUM_27

#define CAMERA_Y9               GPIO_NUM_35
#define CAMERA_Y8               GPIO_NUM_34
#define CAMERA_Y7               GPIO_NUM_39
#define CAMERA_Y6               GPIO_NUM_36
#define CAMERA_Y5               GPIO_NUM_21
#define CAMERA_Y4               GPIO_NUM_19
#define CAMERA_Y3               GPIO_NUM_18
#define CAMERA_Y2               GPIO_NUM_5

#define CAMERA_VSYNC            GPIO_NUM_25
#define CAMERA_HREF             GPIO_NUM_23
#define CAMERA_PCLK             GPIO_NUM_22

// Camera configuration
#define CAMERA_XCLK_FREQ_HZ     20000000
#define CAMERA_TIMER            LEDC_TIMER_0
#define CAMERA_CHANNEL          LEDC_CHANNEL_0

#define CAMERA_PIXEL_FORMAT     PIXFORMAT_JPEG

#define CAMERA_FRAMESIZE        FRAMESIZE_VGA
#define CAMERA_JPEG_QUALITY     12
#define CAMERA_FB_COUNT         2
#define CAMERA_FB_LOCATION      CAMERA_FB_IN_PSRAM

#define CAMERA_GRAB_MODE        CAMERA_GRAB_WHEN_EMPTY


class CameraManager {
// Init camera ----------------------------------------------------------
private:
    CameraManager();

// Camera configuration -------------------------------------------------
private:
    camera_config_t cameraConfig;

// Deinit camera --------------------------------------------------------
public:
    ~CameraManager();

// Singleton ------------------------------------------------------------
private:
    static CameraManager* instance;

public:
    CameraManager(const CameraManager& cameraManager) = delete;

    CameraManager& operator=(const CameraManager& cameraManager) = delete;

    static void init();

    static CameraManager* getInstance() { return instance; }

    static void deinit();
};