#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct{
    uint8_t address[128];
    uint8_t count;
} SPI_DeviceList;

void MX_SPI2_Init(void);

HAL_StatusTypeDef SPI_TransmitReceive(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);
HAL_StatusTypeDef SPI_Receive(uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef SPI_Transmit(uint8_t *pData, uint16_t Size);