#include "i2c.h"
#include "main.h"


I2C_HandleTypeDef hi2c1;

void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

uint8_t enterI2Caddress(UART_HandleTypeDef *huart2){

  char requestAddress[] = "Enter I2C address: ";
  HAL_UART_Transmit(huart2, (uint8_t *)requestAddress, strlen(requestAddress), HAL_MAX_DELAY);
  rearm_uart();
}


HAL_StatusTypeDef I2Cscan(I2C_DeviceList *devices)
{
    if (devices == NULL) {
        return HAL_ERROR;
    }
    
    devices->count = 0;

    for (int i = 1; i < 128; i++) {
        HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&hi2c1, i, 1, 10);
        if (status == HAL_OK) {
            devices->address[devices->count] = i;
            devices->count++;
        }
    }

    return HAL_OK;

}

HAL_StatusTypeDef I2C_ReadMemory(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size) {
    return HAL_I2C_Mem_Read(hi2c, DevAddress, MemAddress, MemAddSize, pData, Size, 1000);
}

HAL_StatusTypeDef I2C_ReadData(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size) {
    return HAL_I2C_Mem_Read(hi2c, DevAddress, MemAddress, MemAddSize, pData, Size, 1000);

}

