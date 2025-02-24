#include "uartHandler.h"
#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <main.h>
#include "i2c.h"
#include <string.h> 



#define CMD_BUFFER_SIZE 32
#define RX_BUFFER_SIZE 128

uint8_t rx_byte;
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint16_t rx_index = 0;
uint8_t command_ready = 0;

UART_HandleTypeDef huart2;

void uart_transmit(void *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	    {
	        // Echo the received character
	        if (rx_byte == '\r' || rx_byte == '\n')
	        {
	            // Only set command ready if we have received some data
	            if (rx_index > 0) {
	                rx_buffer[rx_index] = '\0';  // Null terminate the string
	                command_ready = 1;
	            }

	            // If this is CR, prepare to ignore following LF
	            if (rx_byte == '\r') {
	                HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
	            }
	        }
	        else if (rx_index < RX_BUFFER_SIZE - 1)  // Leave space for null terminator
	        {
	            // Add character to buffer
	            rx_buffer[rx_index++] = rx_byte;

	            // Restart reception for next character
	            HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
	        }
	        else
	        {
	            // Buffer overflow - reset buffer
	            rx_index = 0;
	            memset(rx_buffer, 0, RX_BUFFER_SIZE);

	            // Restart reception
	            HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
	        }
	    }
	    }

void rearm_uart(void)
{
  char msg[] = "stm32> ";
  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
  HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
}

void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
    Error_Handler();
    }

}

void UART_ProcessCommands(void){
    if (command_ready){
    
      command_dispatch();

      memset(rx_buffer, 0, RX_BUFFER_SIZE);  // Clear the buffer
      rx_index = 0;
      command_ready = 0;

      HAL_UART_Receive_IT(&huart2, &rx_byte, 1);  // Restart reception
    }
}

void print_menu(void)
{
    char *menu_text = "Available Commands:\r\n"
                      "1. SCAN - Scan for devices\r\n"
                      "2. READ - Read from device\r\n"
                      "3. MEM_READ - Read from device memory\r\n"
                      "4. WRITE - Write to device\r\n"
                      "5. MEM_WRITE- Write to device memory\r\n"
                      "6. SET_SPEED - Set communication speed\r\n"
                      "7. INFO - Display device information\r\n"
                      "8. RESET - Reset I2C bus\r\n"
                      "9. HELP - Display this menu\r\n"
                      "\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t *)menu_text, strlen(menu_text), HAL_MAX_DELAY);
    rearm_uart();
}

/**
 * @brief      Handle a command from the command line interface
 *
 * @details    This function takes a string from the command line interface
 *             and processes it as a command. The command is given as a
 *             string of characters in the UART receive buffer. The
 *             function should parse the command and handle it.
 *
 * @param[in]  None
 *
 * @return     None
 */
void command_dispatch(void)
{
    int cmd = rx_buffer[0] - '0';

   switch (cmd) {
   case CMD_I2C_SCAN:
    char message[] = "Performing I2C scan...\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
        I2C_DeviceList devices;
        // This function performs the scan and stores list of devices in devices struct 
        if (I2Cscan(&devices) == HAL_OK) {
        	if (devices.count)
				for (int i = 0; i < devices.count; i++) {
					char formattedMessage[64];
					sprintf(formattedMessage, "Device %d: 0x%02X\r\n", i, devices.address[i]);
					HAL_UART_Transmit(&huart2, (uint8_t*)formattedMessage, strlen(formattedMessage), HAL_MAX_DELAY);
				}
        	else{
        		char message[] = "No devices found\r\n";
        		HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            }
            rearm_uart();
        }
        break;
//    case CMD_I2C_READ:
//        // Handle READ command
//         break;
//    case CMD_I2C_MEM_READ:
//        // Handle WRITE command
//        break;
//    case CMD_I2C_WRITE:
//        // Handle SET_SPEED command
//        break;
//    case CMD_I2C_MEM_WRITE:
//        // Handle CONFIGURE command
//        break;
//    case 6:
//        // Handle INFO command
//        break;
//    case 7:
//     break;
//    case 8:
//        // Handle RESET command
//        break;
   case 9:
       print_menu();
       rearm_uart();
       break;
    default:
        char invalid_message[] = "Invalid command\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)invalid_message, strlen(invalid_message), HAL_MAX_DELAY);
        rearm_uart();
    }
}
