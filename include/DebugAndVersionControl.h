/*
 * File: DebugAndVersion.h
 * Project: drone_r6_fw
 * File Created: Sunday, 16th February 2025 10:45:36 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Monday, 17th February 2025 2:29:25 am
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#pragma once

// For Debugging
#define DEBUG_MODE // Comment to disable debug mode

#ifdef DEBUG_MODE
#include "esp_log.h"
#define DEBUG_PRINT(...) ESP_LOGI("DEBUG", __VA_ARGS__)
#define DEBUG_INIT_START(x) ESP_LOGI("DEBUG INIT", "--- Initializing %s ---", x)
#define DEBUG_INIT_END(x) ESP_LOGI("DEBUG INIT", "--- %s initialized ---", x)
#define DEBUG_DEINIT_START(x) ESP_LOGI("DEBUG DEINIT", "--- Deinitializing %s ---", x)
#define DEBUG_DEINIT_END(x) ESP_LOGI("DEBUG DEINIT", "--- %s deinitialized ---", x)
#define DEBUG_INIT_NO_NEED(x) ESP_LOGE("DEBUG INIT", "%s already initialized, or it was never deinitialized.", x)
#define DEBUG_DEINIT_NO_NEED(x) ESP_LOGE("DEBUG DEINIT", "%s already deinitialized, or it was never initialized.", x)
#else
#define DEBUG_PRINT(...)
#define DEBUG_INIT_START(x)
#define DEBUG_INIT_END(x)
#define DEBUG_DEINIT_START(x)
#define DEBUG_DEINIT_END(x)
#define DEBUG_INIT_NO_NEED(x)
#define DEBUG_DEINIT_NO_NEED(x)
#endif

// For Motor Manager
#define MANUAL_CONTROL // Comment to disable manual control and enable auto control

#ifndef MANUAL_CONTROL
#define AUTO_CONTROL
#endif

// For Storage Manager
//#define RESET_MEMORY_TO_DEFAULT // Comment to disable reset memory to default


// For WiFi Modul Manager
#define WIFI_TRY_ATTEMPTS 10

// For Versioning (Comment out the current version to update.)
#define VERSION_ALPHA

#ifndef VERSION_ALPHA
#define VERSION_BETA


#ifndef VERSION_BETA
#define VERSION_1

#ifndef VERSION_1
#define VERSION_2

#ifndef VERSION_2
#define VERSION_3

#endif
#endif
#endif
#endif

#if defined VERSION_BETA || VERSION_1 || VERSION_2 || VERSION_3
#define VERSION_BETA_OR_LATER
#endif

#if defined VERSION_1 || VERSION_2 || VERSION_3
#define VERSION_1_OR_LATER
#endif

#if defined VERSION_2 || VERSION_3
#define VERSION_2_OR_LATER
#endif