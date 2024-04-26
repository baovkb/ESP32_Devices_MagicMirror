typedef enum {
    TypeErr,
    Bulb,
    Door
} TypeDevice;

typedef struct {
    char *name; 
    char *device_id;
    bool active;
} Device_iot;

TypeDevice getTypeDevice(Device_iot *device);
void initDevice(Device_iot *device, const char *name, const char *device_id, bool active);