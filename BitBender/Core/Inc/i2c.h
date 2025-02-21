void MX_I2C1_Init(void);
HAL_StatusTypeDef I2Cscan(I2C_DeviceList *deviceList);

typedef struct{
    uint8_t address[128];
    uint8_t count;
} I2C_DeviceList;