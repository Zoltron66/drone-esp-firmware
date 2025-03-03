/*
 * File: DistanceSensorManager.h
 * Project: drone_r6_fw
 * File Created: Wednesday, 19th February 2025 8:55:34 am
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Wednesday, 19th February 2025 8:55:34 am
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#pragma once

#include "DebugAndVersionControl.h"

#ifdef VERSION_BETA_OR_LATER
// Distance Sensor GPIO pins 
#define DIST_TRIG               12   // GPIO_12  (SD - Data2)
#define DIST_ECHO               13   // GPIO_13  (SD - Data3)


// TODO: Future implementation: Create a distance sensor manager for the beta version (leaser sensor).
#endif