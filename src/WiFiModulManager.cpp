/*
 * File: WiFiModulManager.cpp
 * Project: drone_r6_fw
 * File Created: Wednesday, 19th February 2025 6:09:58 pm
 * Author: MZoltan (zoltan.matus.smm@gmail.com)
 * 
 * Last Modified: Wednesday, 19th February 2025 6:09:58 pm
 * Version: 0.1.0 (ALPHA)
 * 
 * Copyright (c) 2025 MZoltan
 * License: MIT License
 */

#include "WiFiModulManager.h"
#include "LedManager.h"

#include <arpa/inet.h>
#include <lwip/inet.h>

// Init wifi ------------------------------------------------------------
static void WiFiEventCallback(void* arg, esp_event_base_t base, int32_t id, void* data) {
    DEBUG_PRINT("Event base: %s", base);
    DEBUG_PRINT("Event id: 0x%" PRIx32, id);
    
    WiFiModulManager* wifiModulManager = WiFiModulManager::getInstance();

    if (base == WIFI_EVENT) {
        switch (id) {
            case WIFI_EVENT_STA_DISCONNECTED: {
                DEBUG_PRINT("Wi-Fi disconnected");
                if (wifiModulManager->getNetworkStatus() == CONNECTED) {
                    wifiModulManager->setNetworkStatus(DISCONNECTED);
                } else {
                    DEBUG_PRINT("Maintaining network status");
                }
                break;
            }
            case WIFI_EVENT_STA_CONNECTED: {
                DEBUG_PRINT("Wi-Fi connected");
                wifiModulManager->setNetworkStatus(CONNECTED);
                break;
            }
            default: {
                DEBUG_PRINT("Event not handled [Wi-Fi]");
                break;
            }
        }
    } else if (base == IP_EVENT) {
        switch (id) {
            case IP_EVENT_STA_GOT_IP: {
                ip_event_got_ip_t* eventIP = (ip_event_got_ip_t*)data;
                DEBUG_PRINT("Got IP: " IPSTR, IP2STR(&eventIP->ip_info.ip));
                wifiModulManager->geatherGatewayInfos(
                    inet_ntoa(eventIP->ip_info.gw),
                    inet_ntoa(eventIP->ip_info.netmask)
                );
                break;
            }
            default:
                DEBUG_PRINT("Event not handled [IP]");
                break;
        }
    }
}

static esp_netif_t* networkInterface = nullptr;
static esp_event_handler_instance_t wifiEventHandler;
static esp_event_handler_instance_t ipEventHandler;

WiFiModulManager::WiFiModulManager() {
    DEBUG_PRINT("--- Init Wi-Fi called");

    ssid = "";
    password = "";
    networkStatus = DISCONNECTED;
    gatewayIP = "";
    subnetMask = "";
    isStaticIpSet = false;

    esp_err_t err = ESP_OK;
    // ----------------- Initialize Non-Volatile Storage (NVS) -----------------
    err = nvs_flash_init(); // TODO: Initialize Non-Volatile Storage in the StorageManager after this class works
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        DEBUG_PRINT("Failed to initialize NVS");
        ESP_ERROR_CHECK(err);
        return;
    }
    // -------------------------------------------------------------------------
    err = esp_netif_init();
    if (err != ESP_OK) {
        DEBUG_PRINT("Failed to initialize TCP/IP network stack");
        ESP_ERROR_CHECK(err);
        return;
    }
    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        DEBUG_PRINT("Failed to create default event loop");
        ESP_ERROR_CHECK(err);
        return;
    }
    err = esp_wifi_set_default_wifi_sta_handlers();
    if (err != ESP_OK) {
        DEBUG_PRINT("Failed to set default handlers");
        ESP_ERROR_CHECK(err);
        return;
    }

    networkInterface = esp_netif_create_default_wifi_sta();
    if (networkInterface == nullptr) {
        DEBUG_PRINT("Failed to create default WiFi STA interface");
        ESP_ERROR_CHECK(ESP_FAIL);
        return;
    }

    wifi_init_config_t wifiConfig = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiConfig));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFiEventCallback, NULL, &wifiEventHandler));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFiEventCallback, NULL, &ipEventHandler));
    DEBUG_PRINT("Wi-Fi inited ---");
}

// Connect to wifi ------------------------------------------------------
void WiFiModulManager::connectToWiFi() {
    DEBUG_PRINT("--- Connect Wi-Fi called");
    wifi_config_t wifiConfig = {
        .sta = {
            .ssid = "",
            .password = "",
            .scan_method = WIFI_FAST_SCAN,
            .bssid_set = 0,
            .bssid = {0},
            .channel = 0,
            .listen_interval = 0,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold = {
                .rssi = -127,
                .authmode = WIFI_AUTH_WPA2_PSK,
                .rssi_5g_adjustment = 0,
            },
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
            .rm_enabled = 0,
            .btm_enabled = 0,
            .mbo_enabled = 0,
            .ft_enabled = 0,
            .owe_enabled = 0,
            .transition_disable = 0,
            .reserved = 0,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            .sae_pk_mode = WPA3_SAE_PK_MODE_AUTOMATIC,
            .failure_retry_cnt = 0,
            .he_dcm_set = 0,
            .he_dcm_max_constellation_tx = 0,
            .he_dcm_max_constellation_rx = 0,
            .he_mcs9_enabled = 0,
            .he_su_beamformee_disabled = 0,
            .he_trig_su_bmforming_feedback_disabled = 0,
            .he_trig_mu_bmforming_partial_feedback_disabled = 0,
            .he_trig_cqi_feedback_disabled = 0,
            .he_reserved = 0,
            .sae_h2e_identifier = {0}
        }
    };

    strncpy((char*)wifiConfig.sta.ssid, ssid.c_str(), sizeof(wifiConfig.sta.ssid));
    strncpy((char*)wifiConfig.sta.password, password.c_str(), sizeof(wifiConfig.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));

    DEBUG_PRINT("Connecting to Wi-Fi network: %s", wifiConfig.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_connect();
    DEBUG_PRINT("Wi-Fi connecting end---");
}

// Disconnect from wifi -------------------------------------------------
void WiFiModulManager::reconnectToWiFi() {
    DEBUG_PRINT("--- Reconnect Wi-Fi with static IP called");

    if (!isStaticIpSet) {
        esp_netif_ip_info_t ipInfo;
        ipInfo.ip.addr = ipaddr_addr("172.20.10.2");
        ipInfo.gw.addr = ipaddr_addr(gatewayIP.c_str());
        ipInfo.netmask.addr = ipaddr_addr(subnetMask.c_str());

        ESP_ERROR_CHECK(esp_netif_dhcpc_stop(networkInterface)); // Stop DHCP client
        ESP_ERROR_CHECK(esp_netif_set_ip_info(networkInterface, &ipInfo));
        isStaticIpSet = true;
    }
    esp_wifi_connect();
    
    DEBUG_PRINT("Wi-Fi reconnecting end ---");
}

// Disconnect from wifi -------------------------------------------------
void WiFiModulManager::disconnectFromWiFi() {
    DEBUG_PRINT("--- Disconnect Wi-Fi called");
    esp_wifi_disconnect();
    networkStatus = DISCONNECTED;
    DEBUG_PRINT("Wi-Fi disconnected ---");
}

// WiFi Controls --------------------------------------------------------
// Tasks ----------------------------------------------------------------
static void taskWifiControl(void *pvParameters) {
    WiFiModulManager* wifiModulManager = WiFiModulManager::getInstance();
    LedManager* ledManager = LedManager::getInstance();

    wifiModulManager->setNetworkStatus(TRYING_TO_CONNECT);

    int8_t connectionStage = 0;
    int8_t wifiTryAttempts = 0;
    while (true) {
        switch (wifiModulManager->getNetworkStatus()) {
            case DISCONNECTED: {
                DEBUG_PRINT("Wi-Fi disconnected or canceled");
                // If there was a connection, play the disconnected animation
                if (connectionStage == 2 || connectionStage == 1) {
                    connectionStage = 0;
                    ledManager->setAnimation(AnimationType::WIFI_DISCONNECTED);
                }
                // If the connection reachde the maximum attempts, then set the mode to room plant mode
                if (wifiTryAttempts > WIFI_TRY_ATTEMPTS) {
                    // TODO: Impement mode manager, then set it to room plant mode
                    // In room plant mode: Set animation to IDLE and turn everithiong off
                } 
                // If there was a connection attempt, but canceld or there was no connection attemt, then do nothing
                break;
            }
            case TRYING_TO_CONNECT: {
                if (wifiTryAttempts <= WIFI_TRY_ATTEMPTS) {
                    DEBUG_PRINT("Wi-Fi try to connect, attempt: %d", wifiTryAttempts);
                    ledManager->setAnimation(AnimationType::WIFI_CONNECTING);
                    wifiModulManager->connectToWiFi();
                    wifiTryAttempts++;
                } else { wifiModulManager->setNetworkStatus(DISCONNECTED); }
                break;
            }
            case TRYING_TO_RECONNECT: {
                if (wifiTryAttempts <= WIFI_TRY_ATTEMPTS) {
                    DEBUG_PRINT("Wi-Fi try to reconnect, attempt: %d", wifiTryAttempts);
                    ledManager->setAnimation(AnimationType::WIFI_CONNECTING);
                    wifiModulManager->reconnectToWiFi();
                    wifiTryAttempts++;
                } else { wifiModulManager->setNetworkStatus(DISCONNECTED); }
                break;
            }
            case CONNECTED: {
                DEBUG_PRINT("Wi-Fi connected, connection stage: %d", connectionStage);
                if (connectionStage == 0) {
                    if (wifiModulManager->isGatewayInfosEmpty()) {
                        DEBUG_PRINT("Gateway infos are empty, waiting to get the gateway infos...");
                    } else {
                        DEBUG_PRINT("Gateway infos are not empty, setting the gateway infos...");
                        connectionStage = 1;
                        wifiTryAttempts = 0;
                        wifiModulManager->disconnectFromWiFi();
                        wifiModulManager->setNetworkStatus(TRYING_TO_RECONNECT); // Then the task will reconnect
                    }
                } else if (connectionStage == 1) {
                    connectionStage = 2;
                    wifiTryAttempts = 0;
                    ledManager->setAnimation(AnimationType::WIFI_CONNECTED); // Then automatically shows the idle animation
                }
                break;
            }
            default: {
                DEBUG_PRINT("Network status not exist [Not DISCONNECTED, TRYING_TO_CONNECT, TRYING_TO_RECONNECT, CONNECTED]");
                break;
            }
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static TaskHandle_t wifiTaskHandle = nullptr;
void WiFiModulManager::startWiFiControls() {
    xTaskCreatePinnedToCore(&taskWifiControl, "WIF_CONT", 2048, nullptr, 4, &wifiTaskHandle, 1);
}


// Deinitialize the WiFi module -----------------------------------------
WiFiModulManager::~WiFiModulManager() {
    DEBUG_PRINT("--- Deinit Wi-Fi called");
    // BUG: This causes a crash, deleting an instance inside an instance cause stack overflow, because its a deinit loop
    // BUG: Solution: If I create a static deinit method, then I can call it just like this: WiFiModulManager::deinit(); And I can create a flag for a task to delete itself
    // vTaskDelete(wifiTaskHandle);

    // esp_err_t err = esp_wifi_stop();
    // if (err == ESP_ERR_WIFI_NOT_INIT) {
    //     DEBUG_PRINT("Wi-Fi cant be deinitialized, because it is not initialized");
    //     delete instance;
    //     return;
    // }

    // ESP_ERROR_CHECK(esp_wifi_deinit());
    // ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(networkInterface));
    // esp_netif_destroy(networkInterface);

    // ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, ESP_EVENT_ANY_ID, ipEventHandler));
    // ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler));
    // delete instance;
    DEBUG_PRINT("Wi-Fi deinited ---");
}

// Singleton ------------------------------------------------------------
WiFiModulManager* WiFiModulManager::instance = nullptr;

WiFiModulManager* WiFiModulManager::getInstance() {
    if (instance == nullptr) {
        instance = new WiFiModulManager();
    }
    return instance;
}

