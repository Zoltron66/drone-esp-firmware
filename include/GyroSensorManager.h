/*
 * File: GyroSensorManager.h
 * Project: drone_r6_fw
 * File Created: Monday, 17th February 2025 2:46:37 am
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Monday, 17th February 2025 2:46:38 am
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#pragma once

#include "DebugAndVersionControl.h"

#ifdef VERSION_BETA_OR_LATER
// Gyro Sensor GPIO pins (MPU-6050)
#define GYRO_SDA                3   // GPIO_3
#define GYRO_SCL                1   // GPIO_1


// NOTE: Future implementation: Gyro Sensor Manager for Auto-Assisted Controls, stabilization, and orientation detection (mini-compass info).
#endif