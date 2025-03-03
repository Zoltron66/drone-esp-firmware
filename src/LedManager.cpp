/*
 * File: LedManager.cpp
 * Project: drone_r6_fw
 * File Created: Tuesday, 25th February 2025 6:00:34 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Tuesday, 25th February 2025 6:00:34 pm
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#include "LedManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" {
#include <driver/rmt_tx.h>
}

// Colors -------------------------------------------------------------------------------------------------------
// RGB Color ----------------------------------------------------------------------------------------------------
Colors::Color::Color(uint8_t R, uint8_t G, uint8_t B, uint8_t brightness) {
    setR(R);
    setG(G);
    setB(B);
    setBrightness(brightness);
}

// Setters and Getters -----------------------------------------
uint8_t Colors::Color::setOneColor(int color) const {
    color = color > 255 ? 255 : color;
    color = color < 0 ? 0 : color;
    return color;
}

void Colors::Color::setR(int R) {
    R = R > 255 ? 255 : R;
    R = R < 0 ? 0 : R;
    this->R = R; 
}

void Colors::Color::setR(int R, int MIN, int MAX) {
    R = R > MAX ? MAX : R;
    R = R < MIN ? MIN : R;
    setR(R); 
}

void Colors::Color::setG(int G) {
    G = G > 255 ? 255 : G;
    G = G < 0 ? 0 : G;
    this->G = G; 
}

void Colors::Color::setG(int G, int MIN, int MAX) {
    G = G > MAX ? MAX : G;
    G = G < MIN ? MIN : G;
    setG(G);
}

void Colors::Color::setB(int B) {
    B = B > 255 ? 255 : B;
    B = B < 0 ? 0 : B;
    this->B = B;
}

void Colors::Color::setB(int B, int MIN, int MAX) {
    B = B > MAX ? MAX : B;
    B = B < MIN ? MIN : B;
    setB(B);
}

void Colors::Color::setBrightness(int brightness) {
    if (brightness > 100) { brightness = 100; }
    if (brightness < 0) { brightness = 0; }
    this->brightness = brightness; 
}

void Colors::Color::setBrightness(int brightness, int MIN, int MAX) {
    if (brightness > MAX) { brightness = MAX; }
    if (brightness < MIN) { brightness = MIN; }
    setBrightness(brightness);
}

// Compare RGB Colors ------------------------------------------
bool Colors::Color::operator==(const Color &color) const {
    return brightness == color.brightness;
}


// LED -----------------------------------------------------------------------------------------------------------
// LED stages -------------------------------------------------
bool Led::moveTowardsToTargetStage(int speed, int MINB, int MAXB, int MINC, int MAXC) {
    if (currentColorStage == targetColorStage) { return true; } // if finished

    // if the brightness is not the samee
    if (currentColorStage.getBrightness() < targetColorStage.getBrightness()) { currentColorStage.setBrightness(currentColorStage.getBrightness() + 1 * speed, MINB, MAXB); }
    else if (currentColorStage.getBrightness() > targetColorStage.getBrightness()) { currentColorStage.setBrightness(currentColorStage.getBrightness() - 1 * speed, MINB, MAXB); }
    
    // if the red color is not the same
    if (currentColorStage.getRawR() < targetColorStage.getRawR()) { currentColorStage.setR(currentColorStage.getRawR() + 1 * speed, MINC, MAXC); }
    else if (currentColorStage.getRawR() > targetColorStage.getRawR()) { currentColorStage.setR(currentColorStage.getRawR() - 1 * speed, MINC, MAXC); }

    // if the green color is not the same
    if (currentColorStage.getRawG() < targetColorStage.getRawG()) { currentColorStage.setG(currentColorStage.getRawG() + 1 * speed, MINC, MAXC); }
    else if (currentColorStage.getRawG() > targetColorStage.getRawG()) { currentColorStage.setG(currentColorStage.getRawG() - 1 * speed, MINC, MAXC); }

    // if the blue color is not the same
    if (currentColorStage.getRawB() < targetColorStage.getRawB()) { currentColorStage.setB(currentColorStage.getRawB() + 1 * speed, MINC, MAXC); }
    else if (currentColorStage.getRawB() > targetColorStage.getRawB()) { currentColorStage.setB(currentColorStage.getRawB() - 1 * speed, MINC, MAXC); }

    return false; // if not finished
}


// LED Manager ---------------------------------------------------------------------------------------------------
// Init LED manager --------------------------------------------------
#define RMT_LED_RESOLUTION_HZ 10000000

struct RmtLedArrayEncoder {
    rmt_encoder_t baseEncoder;
    rmt_encoder_t *bytesEncoder;
    rmt_encoder_t *copyEncoder;
    int state;
    rmt_symbol_word_t resetCode;
};

static size_t rmtEncodeLedArray(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primaryData, size_t dataSize, rmt_encode_state_t *retState) {
    RmtLedArrayEncoder *ledEncoder = __containerof(encoder, RmtLedArrayEncoder, baseEncoder);
    rmt_encoder_handle_t bytesEncoder = ledEncoder->bytesEncoder;
    rmt_encoder_handle_t copyEncoder = ledEncoder->copyEncoder;
    rmt_encode_state_t sessionState = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    size_t encodedSymbols = 0;
    switch (ledEncoder->state) {
    case 0: // send RGB data
        encodedSymbols += bytesEncoder->encode(bytesEncoder, channel, primaryData, dataSize, &sessionState);
        if (sessionState & RMT_ENCODING_COMPLETE) {
            ledEncoder->state = 1; // switch to next state when current encoding session finished
        }
        if (sessionState & RMT_ENCODING_MEM_FULL) {
            state = static_cast<rmt_encode_state_t>(state | RMT_ENCODING_MEM_FULL);
            *retState = state;
            return encodedSymbols;
        }
    // fall-through
    case 1: // send reset code
        encodedSymbols += copyEncoder->encode(copyEncoder, channel, &ledEncoder->resetCode, sizeof(ledEncoder->resetCode), &sessionState);
        if (sessionState & RMT_ENCODING_COMPLETE) {
            ledEncoder->state = RMT_ENCODING_RESET; // back to the initial encoding session
            state = static_cast<rmt_encode_state_t>(state | RMT_ENCODING_COMPLETE);
        }
        if (sessionState & RMT_ENCODING_MEM_FULL) {
            state = static_cast<rmt_encode_state_t>(state | RMT_ENCODING_MEM_FULL);
            *retState = state;
            return encodedSymbols;
        }
    }
    *retState = state;
    return encodedSymbols;
}

static esp_err_t rmtDeleteLedArrayEncoder(rmt_encoder_t *encoder) {
    RmtLedArrayEncoder *ledEncoder = __containerof(encoder, RmtLedArrayEncoder, baseEncoder);
    rmt_del_encoder(ledEncoder->bytesEncoder);
    rmt_del_encoder(ledEncoder->copyEncoder);
    free(ledEncoder);
    return ESP_OK;
}

static esp_err_t rmtResetLedArrayEncoder(rmt_encoder_t *encoder) {
    RmtLedArrayEncoder *ledEncoder = __containerof(encoder, RmtLedArrayEncoder, baseEncoder);
    rmt_encoder_reset(ledEncoder->bytesEncoder);
    rmt_encoder_reset(ledEncoder->copyEncoder);
    ledEncoder->state = RMT_ENCODING_RESET;
    return ESP_OK;
}

static rmt_channel_handle_t rmtChannelHandle = nullptr;
static rmt_encoder_handle_t rmtEncoderHandle = nullptr;
LedManager::LedManager() {
    DEBUG_PRINT("Initializing LED manager ---");
    // Initialize LEDs
    LED[0] = Led();
    LED[1] = Led();
    LED[2] = Led();
    LED[3] = Led();
    LED[4] = Led();
    LED[5] = Led();
    debugColor = Colors::Off;
    currentColor = Colors::Off;
    currentAnimation = AnimationType::NONE;
    lastAnimation = AnimationType::NONE;
    animationStage = 0;

    // Setup RMT channel
    rmt_tx_channel_config_t channelConfig = {
        .gpio_num = LED_WS2812,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RMT_LED_RESOLUTION_HZ,
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
        .intr_priority = 0,
        .flags = {
            .invert_out = false,
            .with_dma = false,
            .io_loop_back = false,
            .io_od_mode = false,
            .allow_pd = false
        }
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&channelConfig, &rmtChannelHandle));

    // Setup Encoder
    RmtLedArrayEncoder *ledEncoder = nullptr;
    ledEncoder = (RmtLedArrayEncoder*)rmt_alloc_encoder_mem(sizeof(RmtLedArrayEncoder));
    if (ledEncoder == nullptr) {
        DEBUG_PRINT("Failed to allocate memory for RMT encoder");
        return;
    }
    ledEncoder->baseEncoder.encode = rmtEncodeLedArray;
    ledEncoder->baseEncoder.reset = rmtResetLedArrayEncoder;
    ledEncoder->baseEncoder.del = rmtDeleteLedArrayEncoder;
    
    uint16_t duration01 = (uint16_t)(0.3 * RMT_LED_RESOLUTION_HZ / 1000000);
    uint16_t duration10 = (uint16_t)(0.9 * RMT_LED_RESOLUTION_HZ / 1000000);
    rmt_bytes_encoder_config_t bytesEncoderConfig = {
        .bit0 = {
            .duration0 = duration01,
            .level0 = 1,
            .duration1 = duration10,
            .level1 = 0
        },
        .bit1 = {
            .duration0 = duration10,
            .level0 = 1,
            .duration1 = duration01,
            .level1 = 0
        },
        .flags = { .msb_first = true }
    };
    rmt_copy_encoder_config_t encoderConfig = {};
    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&bytesEncoderConfig, &ledEncoder->bytesEncoder));
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&encoderConfig, &ledEncoder->copyEncoder));

    uint16_t reset_ticks = (uint16_t)(RMT_LED_RESOLUTION_HZ / 1000000 * 50 / 2); // reset code duration defaults to 50us
    ledEncoder->resetCode = (rmt_symbol_word_t) {
        .duration0 = reset_ticks,
        .level0 = 0,
        .duration1 = reset_ticks,
        .level1 = 0
    };

    rmtEncoderHandle = &ledEncoder->baseEncoder;

    // Enable RMT channel
    ESP_ERROR_CHECK(rmt_enable(rmtChannelHandle));
    DEBUG_PRINT("--- LED manager initialized");
}

// LED manager controls ----------------------------------------------
bool LedManager::moveAllLedTowardsToTargetColor(int MINB, int MAXB, int MINC, int MAXC) {
    bool finished = true;
    for (uint8_t i = 0; i < 6; i++) {
        if (!LED[i].moveTowardsToTargetStage(animationSpeed, MINB, MAXB, MINC, MAXC)) {
            finished = false;
        }
    }
    return finished;
}

void LedManager::setAllOff() {
    for (uint8_t i = 0; i < 6; i++) {
        LED[i].setOff();
    }
}

// Animation sequences support ---------------------------------------
void LedManager::resetAnimationStageIfChanged(AnimationType animation, uint8_t speed) {
    if (currentAnimation != lastAnimation) {
        lastAnimation = animation;
        animationSpeed = speed;
        animationStage = 0;
    }
}

void LedManager::playNone() {
    if (currentAnimation != lastAnimation) {
        lastAnimation = AnimationType::NONE;
        animationStage = 0;
        setAllOff();
    }
}

// Animation sequences -----------------------------------------------
#define LED_ANIMATION_IDLE_MIN_BRIGHTNESS 5
#define LED_ANIMATION_IDLE_MAX_BRIGHTNESS 80
#define LED_ANIMATION_IDLE_SPEED 4
void LedManager::playIdleAnimation() {
    resetAnimationStageIfChanged(AnimationType::IDLE, LED_ANIMATION_IDLE_SPEED);
    switch (animationStage) {
        case 0: // Initialize the animation
            for (uint8_t i = 0; i < 6; i++) {
                LED[i].setColor(currentColor);
                LED[i].setCurrentColorBrightness(LED_ANIMATION_IDLE_MIN_BRIGHTNESS);
                LED[i].setTargetColorBrightness(LED_ANIMATION_IDLE_MAX_BRIGHTNESS);
            }
            animationStage = 1;
            break;

        case 1:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_IDLE_MIN_BRIGHTNESS, LED_ANIMATION_IDLE_MAX_BRIGHTNESS)) {
                for (uint8_t i = 0; i < 6; i++) {
                    LED[i].setTargetColorBrightness(LED_ANIMATION_IDLE_MIN_BRIGHTNESS);
                }
                animationStage = 2;
            }
            break;

        case 2:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_IDLE_MIN_BRIGHTNESS, LED_ANIMATION_IDLE_MAX_BRIGHTNESS)) {
                for (uint8_t i = 0; i < 6; i++) {
                    LED[i].setTargetColorBrightness(LED_ANIMATION_IDLE_MAX_BRIGHTNESS);
                }
                animationStage = 1;
            }
            break;
    }
}

#ifdef VERSION_1_OR_LATER
void LedManager::playIdleDebugAnimation() {
    resetAnimationStageIfChanged(AnimationType::IDLE_DEBUG, LED_ANIMATION_IDLE_SPEED);
    switch (animationStage) {
        case 0:
            for (uint8_t i = 0; i < 5; i++) {
                LED[i].setColor(currentColor);
                LED[i].setCurrentColorBrightness(LED_ANIMATION_IDLE_MIN_BRIGHTNESS);
                LED[i].setTargetColorBrightness(LED_ANIMATION_IDLE_MAX_BRIGHTNESS);
            }
            LED[5].setColor(debugColor);
            LED[5].setCurrentColorBrightness(100);
            animationStage = 1;
            break;

        case 1:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_IDLE_MIN_BRIGHTNESS, LED_ANIMATION_IDLE_MAX_BRIGHTNESS)) {
                for (uint8_t i = 0; i < 5; i++) {
                    LED[i].setTargetColorBrightness(LED_ANIMATION_IDLE_MIN_BRIGHTNESS);
                }
                LED[5].setColor(debugColor);
                animationStage = 2;
            }
            break;

        case 2:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_IDLE_MIN_BRIGHTNESS, LED_ANIMATION_IDLE_MAX_BRIGHTNESS)) {
                for (uint8_t i = 0; i < 5; i++) {
                    LED[i].setTargetColorBrightness(LED_ANIMATION_IDLE_MAX_BRIGHTNESS);
                }
                LED[5].setColor(debugColor);
                animationStage = 1;
            }
            break;
    }
}
#endif

#define LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS 0
#define LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS 80
#define LED_ANIMATION_WIFI_CONNECTING_SPEED 12
void LedManager::playWifiConnectingAnimation() {
    resetAnimationStageIfChanged(AnimationType::WIFI_CONNECTING, LED_ANIMATION_WIFI_CONNECTING_SPEED);
    switch (animationStage) {
        case 0:
            LED[0].setColor(Colors::Wifi);
            LED[0].setCurrentColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
            LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
            for (uint8_t i = 1; i < 5; i++) {
                LED[i].setColor(Colors::Wifi);
                LED[i].setCurrentColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[i].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
            }
            LED[5].setColor(Colors::Wifi);
            LED[5].setCurrentColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
            LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
            animationStage = 1;
            break;

        case 1:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                animationStage = 2;
            }
            break;

        case 2:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                animationStage = 3;
            }
            break;

        case 3:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                animationStage = 4;
            }
            break;

        case 4:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                animationStage = 1;
            }
            break;
    }
}


#define LED_ANIMATION_WIFI_CONNECTED_SPEED 16
void LedManager::playWifiConnectedAnimation() {
    resetAnimationStageIfChanged(AnimationType::WIFI_CONNECTED, LED_ANIMATION_WIFI_CONNECTED_SPEED);
    switch (animationStage) {
        case 0:
            LED[0].setColor(Colors::Wifi);
            LED[0].setCurrentColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
            LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
            for (uint8_t i = 1; i < 5; i++) {
                LED[i].setColor(Colors::Wifi);
                LED[i].setCurrentColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[i].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
            }
            LED[5].setColor(Colors::Wifi);
            LED[5].setCurrentColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
            LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
            animationStage = 1;
            break;

        case 1:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColor(Colors::Color(10U, 150U, 200U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[1].setTargetColor(Colors::Color(10U, 150U, 200U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[2].setTargetColor(Colors::Color(10U, 150U, 200U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[3].setTargetColor(Colors::Color(10U, 150U, 200U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[4].setTargetColor(Colors::Color(10U, 150U, 200U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[5].setTargetColor(Colors::Color(10U, 150U, 200U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                animationStage = 2;
            }
            break;

        case 2:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColor(Colors::Color(10U, 175U, 175U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[1].setTargetColor(Colors::Color(10U, 175U, 175U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[2].setTargetColor(Colors::Color(10U, 175U, 175U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[3].setTargetColor(Colors::Color(10U, 175U, 175U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[4].setTargetColor(Colors::Color(10U, 175U, 175U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[5].setTargetColor(Colors::Color(10U, 175U, 175U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                animationStage = 3;
            }
            break;

        case 3:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColor(Colors::Color(10U, 200U, 150U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[1].setTargetColor(Colors::Color(10U, 200U, 150U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[2].setTargetColor(Colors::Color(10U, 200U, 150U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[3].setTargetColor(Colors::Color(10U, 200U, 150U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[4].setTargetColor(Colors::Color(10U, 200U, 150U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[5].setTargetColor(Colors::Color(10U, 200U, 150U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                animationStage = 4;
            }
            break;

        case 4:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColor(Colors::Color(10U, 225U, 125U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[1].setTargetColor(Colors::Color(10U, 225U, 125U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[2].setTargetColor(Colors::Color(10U, 225U, 125U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[3].setTargetColor(Colors::Color(10U, 225U, 125U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[4].setTargetColor(Colors::Color(10U, 225U, 125U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[5].setTargetColor(Colors::Color(10U, 225U, 125U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                animationStage = 5;
            }
            break;

        case 5:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[1].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[2].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[3].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[4].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[5].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                animationStage = 6;
            }
            break;
        
        case 6:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[1].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[2].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[3].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[4].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                LED[5].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS));
                animationStage = 7;
            }
            break;

        case 7:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[1].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[2].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[3].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[4].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                LED[5].setTargetColor(Colors::Color(10U, 250U, 100U, LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS));
                animationStage = 8;
            }
            break;
        case 8:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                setAnimation(AnimationType::IDLE);
            }
            break;
    }
}

#define LED_ANIMATION_WIFI_DISCONNECTED_SPEED 35
void LedManager::playWifiDisconnectedAnimation() {
    resetAnimationStageIfChanged(AnimationType::WIFI_DISCONNECTED, LED_ANIMATION_WIFI_DISCONNECTED_SPEED);
    switch (animationStage) { // Wifi(10U, 100U, 250U) -> Orange(150U, 25U, 10U)
        case 0:
            LED[0].setColor(Colors::Error);
            LED[0].setCurrentColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
            LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
            for (uint8_t i = 1; i < 6; i++) {
                LED[i].setColor(Colors::Error);
                LED[i].setCurrentColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[i].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
            }
            animationStage = 1;
            break;

        case 1:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                animationStage = 2;
            }
            break;
        
        case 2:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                animationStage = 3;
            }
            break;

        case 3:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                animationStage = 4;
            }
            break;
        
        case 4:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                animationStage = 5;
            }
            break;

        case 5:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                animationStage = 6;
            }
            break;
        
        case 6:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                animationStage = 7;
            }
            break;

        case 7:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                animationStage = 8;
            }
            break;
        
        case 8:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                animationStage = 9;
            }
            break;

        case 9:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                animationStage = 10;
            }
            break;
        
        case 10:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                animationStage = 11;
            }
            break;

        case 11:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                animationStage = 12;
            }
            break;
        
        case 12:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS);
                animationStage = 13;
            }
            break;

        case 13:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                LED[0].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[1].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[2].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[3].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[4].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                LED[5].setTargetColorBrightness(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS);
                animationStage = 14;
            }
            break;

        case 14:
            if (moveAllLedTowardsToTargetColor(LED_ANIMATION_WIFI_CONNECTING_MIN_BRIGHTNESS, LED_ANIMATION_WIFI_CONNECTING_MAX_BRIGHTNESS)) {
                setAnimation(AnimationType::NONE);
            }
            break;
    }
}

// LED Array Controls --------------------------------------------------------
void LedManager::transmitWaveformToLedArray() {
    uint8_t ledPixels[18]; // 6 LEDs * 3 colors
    for (uint8_t i = 0; i < 6; i++) {
        ledPixels[i * 3] = LED[i].currentColorStage.getG();
        ledPixels[i * 3 + 1] = LED[i].currentColorStage.getR();
        ledPixels[i * 3 + 2] = LED[i].currentColorStage.getB();
    }
    // write all the pixels to the LED array

    rmt_transmit_config_t transmitConfig = {
        .loop_count = 0,
        .flags = {
            .eot_level = 0,
            .queue_nonblocking = 0
        }
    };

    ESP_ERROR_CHECK(rmt_transmit(rmtChannelHandle, rmtEncoderHandle, ledPixels, sizeof(ledPixels), &transmitConfig));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(rmtChannelHandle, portMAX_DELAY));
}

// Tasks -------------------------------------------------------------
static void taskLedArrayControls(void *pvParameters) {
    LedManager *ledManager = LedManager::getInstance();
    while (true) {
        if (!ledManager) {
            vTaskDelete(NULL);
        }
        // Animation
        switch (ledManager->getCurrentAnimation()) {
            case AnimationType::IDLE:
                ledManager->playIdleAnimation();
                break;
            case AnimationType::IDLE_DEBUG:
#ifdef VERSION_1_OR_LATER
                ledManager->playIdleDebugAnimation();
#endif
                break;
            case AnimationType::WIFI_CONNECTING:
                ledManager->playWifiConnectingAnimation();
                break;
            case AnimationType::WIFI_CONNECTED:
                ledManager->playWifiConnectedAnimation();
                break;
            case AnimationType::WIFI_DISCONNECTED:
                ledManager->playWifiDisconnectedAnimation();
                break;
            default:
                ledManager->playNone();
                break;
        }
        ledManager->transmitWaveformToLedArray();
        vTaskDelay(40 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static TaskHandle_t ledTaskHandle = nullptr;
void LedManager::startLedArrayControls() {
    xTaskCreatePinnedToCore(&taskLedArrayControls, "LED_CONT", 4096, nullptr, 4, &ledTaskHandle, 1);
}

// Deinit LED manager --------------------------------------------------------
LedManager::~LedManager() {
    DEBUG_DEINIT_START("LED manager");
    rmt_disable(rmtChannelHandle);
    rmt_del_encoder(rmtEncoderHandle);
    rmt_del_channel(rmtChannelHandle);
    DEBUG_DEINIT_END("LED manager");
}

// Singleton -------------------------------------------------------------
LedManager* LedManager::instance = nullptr;

void LedManager::init() {
    if (instance == nullptr) {
        instance = new LedManager();
        return;
    }
    DEBUG_INIT_NO_NEED("LED manager");
}

void LedManager::deinit() {
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
        vTaskDelete(ledTaskHandle);
        return;
    }
    DEBUG_DEINIT_NO_NEED("LED manager");
}
