#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct{
    uint8_t address[128];
    uint8_t count;
} I2C_DeviceList;

void MX_I2C1_Init(void);
HAL_StatusTypeDef I2Cscan(I2C_DeviceList *deviceList);
