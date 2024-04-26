#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "device_iot.h"

TypeDevice getTypeDevice(Device_iot *device) {
    if (strstr(device->device_id, "bulb_")) {
        return Bulb;
    } else if (strstr(device->device_id, "door_")) {
        return Door;
    } else {
        return TypeErr;
    }
}

void initDevice(Device_iot *device, const char *name, const char *device_id, bool active) {
    device->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(device->name, name);
    device->device_id = (char *)malloc(sizeof(char) * (strlen(device_id) + 1));
    strcpy(device->device_id, device_id);
    device->active = active;
}