#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "Wifi.h"
#include "nvs_flash.h"
#include "esp_websocket_client.h"
#include <cJSON.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include <inttypes.h>
#include <time.h>
#include "device_iot.h"

#define NUM_BULB 1
#define NUM_DOOR 0
#define Bulb_1 2

static char *TAG = "main";

esp_websocket_client_handle_t client;
Device_iot *devicesList;

void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void websocket_app_start();
cJSON *payload(char *action, bool all);
void sendRequest(char *action, bool all);
void handleAction(char *action, cJSON *data);
Device_iot* generateDeviceInfo(uint8_t numBulb, uint8_t numDoor);

void app_main(void)
{
    //Config button
    gpio_reset_pin(Bulb_1);
    gpio_set_direction(Bulb_1,GPIO_MODE_OUTPUT);

    vTaskDelay(200/ portTICK_PERIOD_MS);

    //generate device info
    devicesList = generateDeviceInfo(NUM_BULB, NUM_DOOR);

    //Config connect Webserver
    const char *ssid = "Loading...!";
    const char *password = "29061971";
    const char *gateway = "ws://192.168.1.2:9090";

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //require event loop to check wifi and websocket event
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    connectWifi(ssid, password);
    websocket_app_start(gateway);
}

Device_iot* generateDeviceInfo(uint8_t numBulb, uint8_t numDoor) {
    Device_iot *devicesList = (Device_iot *)malloc(sizeof(Device_iot) * (numBulb + numDoor));

    if (!devicesList) {
        ESP_LOGI(TAG, "allocate memory for device array failed");
        exit(-1);
    }

    uint8_t i;
    for(i=0; i< numBulb; ++i){
        char tmp_name[20], tmp_device_id[20];
        sprintf(tmp_name, "bulb %d", i+1);
        sprintf(tmp_device_id, "bulb_%02d", i);

        initDevice(devicesList + i, tmp_name, tmp_device_id, false);
    }

    for (; i < numBulb + numDoor; ++i) {
        char tmp_name[20], tmp_device_id[20];
        sprintf(tmp_name, "door %d", i-numBulb+1);
        sprintf(tmp_device_id, "door_%02d", i-numBulb);

        initDevice(devicesList + i, tmp_name, tmp_device_id, false);
    }

    return devicesList;
}

void websocket_app_start(const char *gateway) {
    esp_websocket_client_config_t websocket_cfg = {};
    websocket_cfg.uri = gateway;
    websocket_cfg.reconnect_timeout_ms = 2000;

    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
}

void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;

    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Websocket connected");
        sendRequest("join", true);
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Websocket disconnected");
        break;
    case WEBSOCKET_EVENT_DATA:
        cJSON *json;
        json = cJSON_Parse(data->data_ptr);
        if (data->data_len == 0 || json == NULL) {
            return;
        }
        if (cJSON_IsObject(json)) {
            cJSON *sender = cJSON_GetObjectItemCaseSensitive(json, "sender");
            if (sender != NULL && cJSON_IsString(sender) && !strcmp(sender->valuestring, "server")) {

                ESP_LOGI(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);

                cJSON *action = cJSON_GetObjectItemCaseSensitive(json, "action");
                cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
                if (action != NULL && cJSON_IsString(action)) {
                    handleAction(action->valuestring, data);
                }
            }
        }

        cJSON_Delete(json);
        break;
    }
}

cJSON *payload(char *action, bool all) {
    //Data: name, device, active
    cJSON *json, *deviceArr;
    json = cJSON_CreateObject();
    deviceArr = cJSON_CreateArray();

    if (all) {
        for (uint8_t i = 0; i < NUM_BULB + NUM_DOOR; ++i) {
            cJSON *device_data = cJSON_CreateObject();

            cJSON_AddItemToObject(device_data, "active", cJSON_CreateBool(devicesList[i].active));
            cJSON_AddStringToObject(device_data, "device_id", devicesList[i].device_id);
            cJSON_AddStringToObject(device_data, "name", devicesList[i].name);
            
            cJSON_AddItemToArray(deviceArr, device_data);
        } 
    }

    cJSON_AddItemToObject(json, "data", deviceArr);

    cJSON_AddStringToObject(json, "action", action);
    cJSON_AddStringToObject(json, "sender", "devices");

    char *js = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    // ESP_LOGI(TAG, "deviceList[%d]", 0);
    // ESP_LOGI(TAG, "name: %s", devicesList[0].name);
    // ESP_LOGI(TAG, "id: %s", devicesList[0].device_id);
    // ESP_LOGI(TAG, "active: %d", devicesList[0].active);

    return js;
}

void sendRequest(char *action, bool all) {
    if (esp_websocket_client_is_connected(client)) {
      const char *json = payload(action, all);
      esp_websocket_client_send_text(client, json, strlen(json), portMAX_DELAY);
    }
}


void handleAction(char *action, cJSON *data) {
    if (!strcmp(action, "update")) {
        sendRequest("accept", true);
    } else if (!strcmp(action, "request")) {
        for (uint8_t i=0;i<cJSON_GetArraySize(data);i++) {
		    cJSON *subitem=cJSON_GetArrayItem(data,i);

            if (subitem) {
                const char *device_id_request = cJSON_GetObjectItemCaseSensitive(subitem, "device_id")->valuestring;
                bool active_request = cJSON_GetObjectItemCaseSensitive(subitem, "active")->valueint;

                for (uint8_t indDevice = 0; indDevice<NUM_BULB+NUM_DOOR; ++indDevice) {
                    if (!strcmp(device_id_request, devicesList[indDevice].device_id)) {
                        if (active_request != devicesList[indDevice].active) {
                            if (getTypeDevice(devicesList+indDevice) == Bulb) {
                                //handle turn on/off bulb
                                gpio_set_level(Bulb_1, active_request);
                                devicesList[indDevice].active = active_request;

                            } else if (getTypeDevice(devicesList+indDevice) == Door) {

                            }

                            sendRequest("accept", true);

                            break;
                        }
                    }
                }
            }
	    }
    }
}


void checkStateTask() {
    while(1) {

        
    }

    vTaskDelete( NULL );
}
