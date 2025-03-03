/*
 * File: motorManager.h
 * Project: drone_r6_fw
 * File Created: Sunday, 16th February 2025 3:53:26 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Monday, 17th February 2025 2:31:59 am
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#pragma once

#include "DebugAndVersionControl.h"
#include "driver/ledc.h" // LEDC driver for PWM control

// Motor GPIO pins
#define MOTOR_2_GPIO_CW         GPIO_NUM_12
#define MOTOR_2_GPIO_CCW        GPIO_NUM_13
#define MOTOR_1_GPIO_CW         GPIO_NUM_15
#define MOTOR_1_GPIO_CCW        GPIO_NUM_14

// Channels for the motors
#define MOTOR_2_CW              LEDC_CHANNEL_1
#define MOTOR_2_CCW             LEDC_CHANNEL_2
#define MOTOR_1_CW              LEDC_CHANNEL_3
#define MOTOR_1_CCW             LEDC_CHANNEL_4

// Motor configurations
#define MOTOR_TIMER             LEDC_TIMER_1
#define MOTOR_SPEED_MODE        LEDC_LOW_SPEED_MODE
#define MOTOR_DUTY_RES          LEDC_TIMER_8_BIT
#define MOTOR_FREQ              100

// Motor speed limits
#define MOTOR_MAX_OFFSET_SPEED  255 // Absolute Maximum working speed (8 bit)
#define MOTOR_MAX_SPEED         230 // Relative Maximum working speed (8 bit)
#define MOTOR_MIN_SPEED         70  // Minimum working speed (8 bit)
#define MOTOR_OFF               0   // 0% duty cycle (8 bit)


// Control Data -------------------------------------------------------------------------------------------------
struct ControlData {
    int16_t X;
    int16_t Y;
    int8_t L;
    int8_t R;
};

// Motor --------------------------------------------------------------------------------------------------------
class Motor {
// Init motor -----------------------------------------------------------
public:
    /**
     * @brief Construct a new Motor object.
     *
     * @param motorCW LEDC channel for the motor in the clockwise direction.
     * @param motorCCW LEDC channel for the motor in the counter-clockwise direction.
     */
    Motor(int8_t motorCWPin = MOTOR_1_GPIO_CW, ledc_channel_t motorCW = MOTOR_1_CW, int8_t motorCCWPin = MOTOR_1_GPIO_CCW, ledc_channel_t motorCCW = MOTOR_1_CCW);

// Speed and motor channels ----------------------------------------------
private:
    ledc_channel_t motorCW;
    ledc_channel_t motorCCW;
    ledc_timer_config_t timerConfig;
    ledc_channel_config_t channelConfig;
    int16_t speed;

// Setters and Getters ---------------------------------------------------
public:
#ifdef VERSION_ALPHA
    /**
     * @brief Set the speed of the motor.
     * 
     * @param speed Speed of the motor (0-100).
     */
    void setSpeed(int16_t speed);
#endif
#ifdef VERSION_BETA_OR_LATER
    /**
     * @brief Set the speed of the motor with an offset.
     * 
     * @param speed Speed of the motor (0-100).
     * @param offset Offset for the speed of the motor.
     */
    void setSpeed(int16_t speed, int16_t offset);
#endif
    int16_t getSpeed() const { return speed; }

// Deinit motor ----------------------------------------------------------
public:
    ~Motor();

// Private methods -------------------------------------------------------
private:
    /**
     * @brief Convert the speed percentage to a duty cycle.
     * 
     * @param speed Speed of the motor (0-100).
     * @return int16_t Duty cycle of the motor (70-230).
     */
    int16_t convertSpeedPercentageToDutyCycle(int16_t speed);

    /**
     * @brief Configure the GPIO pins for the motor.
     *
     * @param pin GPIO pin number.
     * @param motor LEDC channel for the motor.
     */
    void motorPinConfig(int8_t pin, ledc_channel_t motor); 
};

// Motor Manager ------------------------------------------------------------------------------------------------
class MotorManager {
// Init motor manager ---------------------------------------------------
private:
    MotorManager();

// Motor controls -------------------------------------------------------
private:
    Motor leftMotor;
    Motor rightMotor;
 
#ifdef MANUAL_CONTROL
    // Full-Manual Control
    /**
     * @brief Move the drone on the X-axis manually.
     * 
     * @param speed Speed of the drone (0-100).
     * @param L Steering of the drone on the Z-axis (0-100).
     * @param R Steering of the drone on the Z-axis (0-100).
     */
    void moveOnYAxisManual(int16_t speed, int8_t L, int8_t R);

    /**
     * @brief Move the drone on the Y-axis manually.
     * 
     * @param speed Speed of the drone (0-100).
     */
    void moveOnXAxisManual(int16_t speed);

    /**
     * @brief Turn on the Z-axis manually.
     * 
     * @param L Steering of the drone on the Z-axis (0-100).
     * @param R Steering of the drone on the Z-axis (0-100).
     */
    void turnOnZAxisManual(int8_t L, int8_t R);
#endif
#ifdef VERSION_BETA_OR_LATER // TODO: Future implementation (Auto-Assisted Controls)
#ifdef AUTO_CONTROL
    // Auto-Assisted Controls (Not Implemented yet) [gyro, accelerometer, etc.]
    void moveOnXAxisAutoAssisted(int16_t speed, int16_t steering);
    void moveOnYAxisAutoAssisted(int16_t speed);
    void moveOnXAxisAutoAssisted(int16_t steering);
#endif
#endif
    void allStop();

// Control --------------------------------------------------------------
private:
    ControlData controlData;

public:
    /**
     * @brief Set the control data for the drone.
     * 
     * @param X Speed of the drone on the X-axis (0-100).
     * @param Y Speed of the drone on the Y-axis (0-100).
     * @param L Steering of the drone on the Z-axis (0-100).
     * @param R Steering of the drone on the Z-axis (0-100).
     */
    void setControlData(int16_t X, int16_t Y, int8_t L, int8_t R);

    ControlData getControlData() const { return controlData; }

#ifdef MANUAL_CONTROL
    void directionControlManual();
#endif
#ifdef VERSION_BETA_OR_LATER
#ifdef AUTO_CONTROL
    void directionControlAutoAssisted();
#endif
#endif
// Motor Controls --------------------------------------------------------
    void startMotorControls();

// Deinit motor manager -------------------------------------------------
public:
    ~MotorManager();

// Singleton -------------------------------------------------------------
private:
    static MotorManager* instance;

public:
    MotorManager(const MotorManager& motorManager) = delete;

    MotorManager& operator=(const MotorManager& motorManager) = delete;

    static MotorManager* getInstance();

    static void deinit() { delete instance; }
};

// DONE: MotorManager.h VERSION_ALPHA
