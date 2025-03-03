/*
 * File: MotorManager.cpp
 * Project: drone_r6_fw
 * File Created: Sunday, 16th February 2025 8:35:02 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Monday, 17th February 2025 2:34:55 am
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#include "MotorManager.h"
#include "driver/ledc.h"        // LEDC driver for PWM control
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" {
#include <string.h>
}

// Motor --------------------------------------------------------------------------------------------------------
// Init motor -----------------------------------------------------------
Motor::Motor(int8_t motorCWPin, ledc_channel_t motorCW, int8_t motorCCWPin, ledc_channel_t motorCCW) {
    this->motorCW = motorCW;
    this->motorCCW = motorCCW;
    motorPinConfig(motorCWPin, motorCW);
    motorPinConfig(motorCCWPin, motorCCW);
    speed = MOTOR_OFF;
}

// Setters and Getters ---------------------------------------------------
#ifdef VERSION_ALPHA
void Motor::setSpeed(int16_t speed) {
    if (speed > 1 || speed < -1) { speed = convertSpeedPercentageToDutyCycle(speed); }
    else { speed = MOTOR_OFF; }
    ledc_set_duty(MOTOR_SPEED_MODE, motorCW, speed > 0 ? speed : 0);
    ledc_set_duty(MOTOR_SPEED_MODE, motorCCW, speed < 0 ? -speed : 0);
    ledc_update_duty(MOTOR_SPEED_MODE, motorCW);
    ledc_update_duty(MOTOR_SPEED_MODE, motorCCW);
}
#endif
#ifdef VERSION_BETA_OR_LATER
    void Motor::setSpeed(int16_t speed, int16_t offset) { }
#endif

// Deinit motor ----------------------------------------------------------
Motor::~Motor() {
    setSpeed(MOTOR_OFF);
    ledc_stop(MOTOR_SPEED_MODE, motorCW, 0);
    ledc_stop(MOTOR_SPEED_MODE, motorCCW, 0);
}

// Private methods -------------------------------------------------------
int16_t Motor::convertSpeedPercentageToDutyCycle(int16_t speed) {
    if (speed > 0) { return int16_t(speed * 1.6 + MOTOR_MIN_SPEED); }
    else { return int16_t(speed * 1.6 - MOTOR_MIN_SPEED); }
}

void Motor::motorPinConfig(int8_t pin, ledc_channel_t motor) {
    DEBUG_PRINT("--- Configuring motor pins");
    memset(&timerConfig, 0, sizeof(ledc_timer_config_t));
    timerConfig = {
        .speed_mode = MOTOR_SPEED_MODE,
        .duty_resolution = MOTOR_DUTY_RES,
        .timer_num = MOTOR_TIMER,
        .freq_hz = MOTOR_FREQ,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = false
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timerConfig));

    memset(&channelConfig, 0, sizeof(ledc_channel_config_t));
    channelConfig = {
        .gpio_num = (int)pin,
        .speed_mode = MOTOR_SPEED_MODE,
        .channel = motor,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = MOTOR_TIMER,
        .duty = 0,
        .hpoint = 0,
        //.sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_ALLOW_PD,
        .flags = { .output_invert = false }
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channelConfig));
    DEBUG_PRINT("Motor pins configured ---");
}


// MotorManager --------------------------------------------------------------------------------------------------------
// Init motor manager ---------------------------------------------------
MotorManager::MotorManager() {
    leftMotor = Motor(MOTOR_1_GPIO_CW, MOTOR_1_CW, 
                      MOTOR_1_GPIO_CCW, MOTOR_1_CCW);

    rightMotor = Motor(MOTOR_2_GPIO_CW, MOTOR_2_CW, 
                       MOTOR_2_GPIO_CCW, MOTOR_2_CCW);

    controlData = { // Initialize control data
        .X = 0,
        .Y = 0,
        .L = 0,
        .R = 0
    };
}

// Motor controls -------------------------------------------------------
#ifdef MANUAL_CONTROL
void MotorManager::moveOnYAxisManual(int16_t speed, int8_t L, int8_t R) {
    // Move forward
    if (speed > 0) {
        // Turning left
        if (L > 0 && R == 0) {
            leftMotor.setSpeed(speed - L);  // Adjust speed
            rightMotor.setSpeed(speed);     // Rotate Clockwise
        }
        // Trning right
        else if (R > 0 && L == 0) {
            rightMotor.setSpeed(speed - R); // Adjust speed
            leftMotor.setSpeed(speed);      // Rotate Clockwise
        }
        // Move forward
        else {
            leftMotor.setSpeed(speed);      // Rotate Clockwise
            rightMotor.setSpeed(speed);     // Rotate Clockwise
        }
    }
    // Move backward
    else if (speed < 0) {
        // Turning left
        if (L > 0 && R == 0) {
            rightMotor.setSpeed(speed + L); // Adjust speed
            leftMotor.setSpeed(speed);      // Rotate Counter-Clockwise
        }
        // Turning right
        else if (R > 0 && L == 0) {
            leftMotor.setSpeed(speed + R);  // Adjust speed
            rightMotor.setSpeed(speed);     // Rotate Counter-Clockwise
        }
        // Move backward
        else {
            leftMotor.setSpeed(speed);      // Rotate Counter-Clockwise
            rightMotor.setSpeed(speed);     // Rotate Counter-Clockwise
        }
    }
    else { allStop(); }                     // For safety
}

void MotorManager::moveOnXAxisManual(int16_t speed) {
    // Move right
    if (speed > 0) {
        leftMotor.setSpeed(speed);      // Rotate Clockwise
        rightMotor.setSpeed(-speed);    // Rotate Counter-Clockwise
    }
    // Move left
    else if (speed < 0) {
        leftMotor.setSpeed(speed);      // Rotate Counter-Clockwise
        rightMotor.setSpeed(-speed);    // Rotate Clockwise
    }
    // Stop
    else { allStop(); }                 // For safety
}

void MotorManager::turnOnZAxisManual(int8_t L, int8_t R) {
    // Rotate left
    if (L > 0 && R == 0) {
        rightMotor.setSpeed(L);         // Rotate Clockwise
        leftMotor.setSpeed(MOTOR_OFF);  // Motor off
    }
    // Rotate right
    else if (R > 0 && L == 0) {
        leftMotor.setSpeed(R);          // Rotate Clockwise
        rightMotor.setSpeed(MOTOR_OFF); // Motor off
    }
    // Stop
    else { allStop(); }                 // For safety
}
#endif
#ifdef VERSION_BETA_OR_LATER // Beta update (Auto-Assisted Controls)
#ifdef AUTO_CONTROL
void MotorManager::moveOnXAxisAutoAssisted(int16_t speed, int16_t steering) { }

void MotorManager::moveOnYAxisAutoAssisted(int16_t speed) { }

void MotorManager::moveOnXAxisAutoAssisted(int16_t steering) { }

#endif
#endif

void MotorManager::allStop() {
    leftMotor.setSpeed(MOTOR_OFF);
    rightMotor.setSpeed(MOTOR_OFF);
}

// Control --------------------------------------------------------------
void MotorManager::setControlData(int16_t X, int16_t Y, int8_t L, int8_t R) {
    controlData.X = X;
    controlData.Y = Y;
    if (L > 0 && R > 0) { L = 0; R = 0; } // For safety
    controlData.L = L;
    controlData.R = R;
}

#ifdef MANUAL_CONTROL
void MotorManager::directionControlManual() {
    // Horizontal movement (Y, L, R = Dont Care)
    if (controlData.X >= 70 || controlData.X <= -70) {
        moveOnXAxisManual(controlData.X);
    }
    // Vertical movement (X < 70 || X > -70; L, R = 0-100)
    else if (controlData.Y >= 2 || controlData.Y <= -2) { 
        moveOnYAxisManual(controlData.Y, controlData.L, controlData.R); 
    }
    // Rotation (X = Dont Care; Y < 2 || -2 > Y; L, R = 0-100)
    else if ((controlData.L > 0 && controlData.R == 0) || (controlData.R > 0 && controlData.L == 0)) {
        turnOnZAxisManual(controlData.L, controlData.R);
    }
    // All stop (X = 0; Y = 0; L = 0; R = 0)
    else { allStop(); }
}
#endif
#ifdef VERSION_BETA_OR_LATER // Beta update (Auto-Assisted Controls)
#ifdef AUTO_CONTROL
void MotorManager::directionControlAutoAssisted() { }

#endif
#endif

// Motor Controls --------------------------------------------------------
// Tasks ----------------------------------------------------------------
static void taskDirectionControl(void *pvParameters) {
    MotorManager* motorManager = MotorManager::getInstance();
    while (true) {
#ifdef MANUAL_CONTROL
        motorManager->directionControlManual();
#endif
#ifdef VERSION_BETA_OR_LATER
#ifdef AUTO_CONTROL
        motorManager->directionControlAutoAssisted();
#endif
#endif
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static TaskHandle_t motorTaskHandle = nullptr;
void MotorManager::startMotorControls() {
    xTaskCreatePinnedToCore(&taskDirectionControl, "DIR_CONT", 1024, nullptr, 5, &motorTaskHandle, 1);
}

// Deinit motor manager ------------------------------------------------
MotorManager::~MotorManager() {
    // BUG: This causes a crash, deleting an instance inside an instance cause stack overflow, because its a deinit loop
    // vTaskDelete(motorTaskHandle);
    // leftMotor.~Motor();
    // rightMotor.~Motor();
    // delete instance;
}

// Singleton -------------------------------------------------------------
MotorManager* MotorManager::instance = nullptr;

MotorManager* MotorManager::getInstance() {
    if (instance == nullptr) {
        instance = new MotorManager();
    }
    return instance;
}

// DONE: MotorManager.c VERSION_ALPHA