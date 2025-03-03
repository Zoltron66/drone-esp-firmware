/*
 * File: UnitTests.h
 * Project: drone_r6_fw
 * File Created: Wednesday, 26th February 2025 11:23:25 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Wednesday, 26th February 2025 11:23:25 pm
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#pragma once

#define UNIT_TESTS // Comment to disable unit tests

#ifdef UNIT_TESTS

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Includes for tests
#include "LedManager.h"
#include "WiFiModulManager.h"
#include "StorageManager.h"

#define UNIT_PRINT(...) ESP_LOGI("UNIT TEST", __VA_ARGS__)
#define TEST_START(x) UNIT_PRINT("\n--- %s Unit Test Started ---\n", x);
#define TEST_END_PASSED(x) UNIT_PRINT("\n--- %s Unit Test Ended [PASSED] ---\n", x);
#define TEST_END_FAILED(x) UNIT_PRINT("\n--- %s Unit Test Ended [FAILED] ---\n", x);


namespace UnitTests {
    //void CameraManagerUnitTest(bool isLoop);

    //void DistanceSensorManagerUnitTest(bool isLoop);

    //void GyroSensorManagerUnitTest(bool isLoop);

    void LedManagerUnitTest(bool isLoop);

    //void ModeManagerUnitTest(bool isLoop);

    //void MotorManagerUnitTest(bool isLoop);

    //void ServerManagerUnitTest(bool isLoop);

    void StorageManagerUnitTest(bool isLoop);

    void WiFiModulManagerUnitTest(bool isLoop);
}

#endif