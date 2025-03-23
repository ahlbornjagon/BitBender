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
control_flags_t control_flags;
UART_HandleTypeDef huart2;
command_parmas_t command_params;

void uart_transmit(void *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	    {
            /*TODO: Going to need a better way to handle input based on if we want a command, address, or data for 
            various commands*/
	        
            // Echo the received character
	        if (rx_byte == '\r' || rx_byte == '\n')
	        {
	            // Set command ready flag
	            if ((rx_index > 0) && !control_flags.commandReady) {
	                rx_buffer[rx_index] = '\0';  // Null terminate the string
	                control_flags.commandReady = 1;
	            }

                // Check if we are setting address
                if ((rx_index > 0) && control_flags.setAddress) {
                    command_params.address = rx_buffer[0];
                    control_flags.setAddress = 0;
                }
                
                // Check if we are setting data
                if ((rx_index > 0) && control_flags.setData) {
                    command_params.data = rx_buffer[0];
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
  char msg[] = "BitBender> ";
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
    if (control_flags.commandReady) {
    
      command_dispatch();
      clear_uart_buffer();
      control_flags.commandReady = 0;

      HAL_UART_Receive_IT(&huart2, &rx_byte, 1);  // Restart reception
    }
}

void clear_uart_buffer(void) {
    memset(rx_buffer, 0, RX_BUFFER_SIZE);  // Clear the buffer
    rx_index = 0;
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

    /*
    This case statement is for the I2C scan command. Passes in a struct for devices found on the i2c bus to
    be stored inside of.
    */
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

    /*
    This case statement is for the I2C read command. It first sets a read address flag for the uart interrupt 
    so the user entered data is stored as the device address. It clears the buffer, then does the same for the 
    data to be read.
    */
   case CMD_I2C_READ:
    control_flags.setAddress = 1;
    clear_uart_buffer();
    enterI2Caddress(&huart2);
    clear_uart_buffer();
    control_flags.setData = 1;
    enterI2Cdata(&huart2);
    char message[] = "Performing I2C read...\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);

    
       // Handle READ command
        break;
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
       break;
    default:
        char invalid_message[] = "Invalid command\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)invalid_message, strlen(invalid_message), HAL_MAX_DELAY);
        rearm_uart();
    }
}
