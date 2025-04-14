#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct{
    uint8_t address[128];
    uint8_t count;
} I2C_DeviceList;

void MX_I2C1_Init(void);
HAL_StatusTypeDef I2Cscan(I2C_DeviceList *deviceList);
HAL_StatusTypeDef I2C_ReadMemory(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef I2C_WriteMemory(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size); 
HAL_StatusTypeDef I2C_ReadData(uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef I2C_WriteData(uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef I2C_IsDeviceReady(uint16_t DevAddress);
HAL_StatusTypeDef I2C_RESET ();
bool setI2Cspeed(uint8_t speed);