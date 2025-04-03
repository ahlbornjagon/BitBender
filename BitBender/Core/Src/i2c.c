#include "i2c.h"
#include "main.h"
#include <stdbool.h>



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


bool setI2Cspeed(uint8_t speed) {
  // Creates array of valid speeds, then checks if speed is in array
  int VALIDSPEEDS[] = {100000, 400000, 1000000};
  const int NUM_VALID_SPEEDS = sizeof(VALIDSPEEDS) / sizeof(VALIDSPEEDS[0]);

  for (int i = 0; i < NUM_VALID_SPEEDS; i++) {
    if (speed == VALIDSPEEDS[i]) {
        hi2c1.Init.ClockSpeed = speed;
        return true;
      } 
  }
  return false;



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

HAL_StatusTypeDef I2C_ReadMemory(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size) {
  return HAL_I2C_Mem_Read(&hi2c1, DevAddress, MemAddress, MemAddSize, pData, Size, 1000);
}


HAL_StatusTypeDef I2C_WriteMemory(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size) {
  return HAL_I2C_Mem_Write(&hi2c1, DevAddress, MemAddress, MemAddSize, pData, Size, 1000);
}


HAL_StatusTypeDef I2C_ReadData(uint16_t DevAddress, uint8_t *pData, uint16_t Size) {
  return HAL_I2C_Master_Receive(&hi2c1, DevAddress, pData, Size, 1000);
}

HAL_StatusTypeDef I2C_WriteData(uint16_t DevAddress, uint8_t *pData, uint16_t Size) {
  return HAL_I2C_Master_Transmit(&hi2c1, DevAddress, pData, Size, 1000);
}


HAL_StatusTypeDef I2C_IsDeviceReady(uint16_t DevAddress) {
  return HAL_I2C_IsDeviceReady(&hi2c1, DevAddress, 2, 1000);
}

HAL_StatusTypeDef I2C_RESET () {
  HAL_I2C_DeInit(&hi2c1);
  HAL_StatusTypeDef status = HAL_I2C_Init(&hi2c1);
  return status;
}


// Working on master read command. Need to read and return data to be displayed, or just display it here
// uint8_t I2C_Master_RX(void) {
  
//   uint8_t deviceAddress = 0x42; 
//   uint8_t buffer[10];           
//   HAL_StatusTypeDef status;   

//   status = HAL_I2C_Master_Receive(&hi2c1,     // I2C handle
//     (deviceAddress << 1), 
//     buffer,      
//     sizeof(buffer), 
//     HAL_MAX_DELAY); 
// }


