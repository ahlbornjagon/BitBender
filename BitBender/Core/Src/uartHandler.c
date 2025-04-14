#include "uartHandler.h"
#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <main.h>
#include "i2c.h"
#include <string.h> 
#include <stdbool.h>
#include "gpio.h"



#define CMD_BUFFER_SIZE 32
#define RX_BUFFER_SIZE 128
#define MAX_READ_LENGTH 32
#define MAX_WRITE_LENGTH 32

uint8_t rx_byte;
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint16_t rx_index = 0;
bool command_ready = 0;

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
  char msg[] = "bitbender> ";
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
                      "1. i2cscan - Scan for devices\r\n"
                      "2. i2cread - Read from device\r\n"
                      "3. i2creadmem - Read from device memory\r\n"
                      "4. i2cwrite - Write to device\r\n"
                      "5. i2cwritemem - Write to device memory\r\n"
                      "6. i2cspeed - Set communication speed\r\n"
                      "7. info - Display device information\r\n"
                      "8. reset - Reset I2C bus\r\n"
                      "9. help - Display this menu\r\n"
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
 * 
 */

 commands_t ParseCommand(const char* cmd_str) {
    if (strcmp(cmd_str, "i2cscan") == 0) {
        return CMD_I2C_SCAN;
    } else if (strcmp(cmd_str, "i2cwrite") == 0) {
        return CMD_I2C_WRITE;
    } else if (strcmp(cmd_str, "i2cread") == 0) {
        return CMD_I2C_READ;
    } else if (strcmp(cmd_str, "i2cmemread") == 0) {
        return CMD_I2C_MEM_READ;
    } else if (strcmp(cmd_str, "i2cmemwrite") == 0) {
        return CMD_I2C_MEM_WRITE;
    } else if (strcmp(cmd_str, "i2cspeed") == 0) {
        return CMD_I2C_SPEED;
    } else if (strcmp(cmd_str, "i2reset") == 0) {
        return CMD_I2C_RESET;
    } else if (strcmp(cmd_str, "spiread") == 0) {
        return CMD_SPI_READ;
    } else if (strcmp(cmd_str, "spiwrite") == 0) {
        return CMD_SPI_WRITE;
    } else if (strcmp(cmd_str, "spitransfer") == 0) {
        return CMD_SPI_TRANSFER;
    } else if (strcmp(cmd_str, "help") == 0) {
        return CMD_HELP;
    }
    else {
        return 0; // Invalid command
    }
}
void command_dispatch(void)
{
    // Pulls the first word of the command
    char* cmd_str = strtok(rx_buffer, " \t\r\n");
    if (cmd_str == NULL) {
        printf("Error: Empty command\r\n");
        return;
    }
    char message[100];

    // Validate the command against command list
   commands_t cmd = ParseCommand(cmd_str);
   switch (cmd) {

    // Scan for devices
   case CMD_I2C_SCAN:
   {
        strcpy(message, "Performing I2C scan...\r\n");
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
                strcpy(message, "No devices found\r\n");
                HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            }
            rearm_uart();
        }
        break;
    }

    // Read from a device
   case CMD_I2C_READ:
   {
        char* addr_str = strtok(NULL, " \t\r\n");
        if (addr_str == NULL) {
            strcpy(message, "Device address is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        char* size = strtok(NULL, " \t\r\n");
        if (size == NULL) {
            strcpy(message, "Bytes to read is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        uint8_t len = (uint8_t)strtol(size, NULL, 0);
        if (len > MAX_READ_LENGTH) {
            snprintf(message, sizeof(message), "Warning: Read length limited to %d bytes\r\n", MAX_READ_LENGTH);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            len = MAX_READ_LENGTH;
        }
        uint8_t data_buffer[MAX_READ_LENGTH];
        HAL_StatusTypeDef status = I2C_ReadData(addr_str, data_buffer, len);
        if (status == HAL_OK) {
            snprintf(message, sizeof(message), "Read from device 0x%02X", addr_str);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            
            // Print data in rows of 8 bytes
            for (int i = 0; i < len; i++) {
                // Format each byte
                snprintf(message, sizeof(message), "0x%02X ", data_buffer[i]);
                HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
                
                // Add a newline after every 8 bytes
                if ((i + 1) % 8 == 0) {
                    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
                }
            }
            
            // Add final newline if not already added
            if (len % 8 != 0) {
                HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
            }
            rearm_uart();
        } else {
            snprintf(message, sizeof(message), "Error: I2C read failed with status %d\r\n", status);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
        }
        break;
    }
    // Read from a specific register inside a device
    case CMD_I2C_MEM_READ:
    {
        char* addr_str = strtok(NULL, " \t\r\n");
        if (addr_str == NULL) {
            strcpy(message, "Device address is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        uint8_t dev_addr = (uint8_t)strtol(addr_str, NULL, 0);
        
        // Parse the register/memory address
        char* reg_str = strtok(NULL, " \t\r\n");
        if (reg_str == NULL) {
            strcpy(message, "Memory address is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        uint8_t reg_addr = (uint8_t)strtol(reg_str, NULL, 0);
        
        // Parse the number of bytes to read
        char* len_str = strtok(NULL, " \t\r\n");
        if (len_str == NULL) {
            strcpy(message, "Bytes to read is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        uint8_t len = (uint8_t)strtol(len_str, NULL, 0);
        
        if (len > MAX_READ_LENGTH) {
            snprintf(message, sizeof(message), "Warning: Read length limited to %d bytes\r\n", MAX_READ_LENGTH);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            len = MAX_READ_LENGTH;
        }
        
        uint8_t data_buffer[MAX_READ_LENGTH];
        HAL_StatusTypeDef status = I2C_ReadMemory(dev_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, data_buffer, len);
        
        if (status == HAL_OK) {
            snprintf(message, sizeof(message), "Read from device 0x%02X, memory 0x%02X:\r\n", dev_addr, reg_addr);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            
            // Print data in rows of 8 bytes
            for (int i = 0; i < len; i++) {
                // Format each byte
                snprintf(message, sizeof(message), "0x%02X ", data_buffer[i]);
                HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
                
                // Add a newline after every 8 bytes
                if ((i + 1) % 8 == 0) {
                    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
                }
            }
            
            // Add final newline if not already added
            if (len % 8 != 0) {
                HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
            }
            rearm_uart();
        } else {
            snprintf(message, sizeof(message), "Error: I2C memory read failed with status %d\r\n", status);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
        }
        break;
    }
    // Write to a device
    case CMD_I2C_WRITE:{
        char* addr_str = strtok(NULL, " \t\r\n");
        if (addr_str == NULL) {
            strcpy(message, "Device address is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        uint8_t dev_addr = (uint8_t)strtol(addr_str, NULL, 0);
        
        // Parse the register address
        char* reg_str = strtok(NULL, " \t\r\n");
        if (reg_str == NULL) {
            strcpy(message, "Register address is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        uint8_t reg_addr = (uint8_t)strtol(reg_str, NULL, 0);
        
        // Parse the data to write
        char* data_str = strtok(NULL, " \t\r\n");
        if (data_str == NULL) {
            strcpy(message, "Data to write is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        uint8_t data_byte = (uint8_t)strtol(data_str, NULL, 0);
        
        // Perform the I2C write operation
        HAL_StatusTypeDef status = I2C_WriteData(dev_addr, &data_byte, 1);
        
        if (status == HAL_OK) {
            snprintf(message, sizeof(message), "Write 0x%02X to device 0x%02X, register 0x%02X: OK\r\n", 
                     data_byte, dev_addr, reg_addr);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
        } else {
            snprintf(message, sizeof(message), "Error: I2C write failed with status %d\r\n", status);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
        }
        rearm_uart();
        break;
    }
    // Write to a specific memory address of a device
    case CMD_I2C_MEM_WRITE:
    {
        char* addr_str = strtok(NULL, " \t\r\n");
        if (addr_str == NULL) {
            strcpy(message, "Device address is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        uint8_t dev_addr = (uint8_t)strtol(addr_str, NULL, 0);
        
        // Parse the memory address
        char* mem_str = strtok(NULL, " \t\r\n");
        if (mem_str == NULL) {
            strcpy(message, "Memory address is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        uint16_t mem_addr = (uint16_t)strtol(mem_str, NULL, 0);
        
        // Parse the data to write
        char* data_str = strtok(NULL, " \t\r\n");
        if (data_str == NULL) {
            strcpy(message, "Data to write is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        uint8_t data_byte = (uint8_t)strtol(data_str, NULL, 0);
        
        // Determine memory address size (8-bit or 16-bit)
        uint8_t mem_addr_size = (mem_addr > 0xFF) ? I2C_MEMADD_SIZE_16BIT : I2C_MEMADD_SIZE_8BIT;
        
        // Perform the I2C memory write operation
        HAL_StatusTypeDef status = I2C_WriteMemory(dev_addr, mem_addr, mem_addr_size, &data_byte, 1);
        
        if (status == HAL_OK) {
            snprintf(message, sizeof(message), "Memory write 0x%02X to device 0x%02X, address 0x%04X: OK\r\n", 
                     data_byte, dev_addr, mem_addr);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
        } else {
            snprintf(message, sizeof(message), "Error: I2C memory write failed with status %d\r\n", status);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
        }
        rearm_uart();
        break;
    }

    // Set I2C speed
    case CMD_I2C_SPEED:
    {
       char* speed_str = strtok(NULL, " \t\r\n");
       uint8_t speed = (uint8_t)strtol(speed_str, NULL, 0);
       bool isvalid = setI2Cspeed(speed);
       if (isvalid) {
            snprintf(message, sizeof(message), "I2C speed set to %d kHz\r\n", speed);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
        } else {
            snprintf(message, sizeof(message), "Invalid I2C speed. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
        }
       break;
    }

    // Reset I2C bus
    case CMD_I2C_RESET:
    {
        HAL_StatusTypeDef status = I2C_RESET();

        if (status == HAL_OK) {
            snprintf(message, sizeof(message), "I2C bus reset: OK\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
        } else {
            snprintf(message, sizeof(message), "Error: I2C bus reset failed with status %d\r\n", status);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
        break;
        }
    }


    case CMD_SPI_WRITE:
    {
        char* cs_pin_str = strtok(NULL, " \t\r\n");
        if (cs_pin_str == NULL) {
            strcpy(message, "CS pin is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        
        uint8_t tx_buffer[MAX_WRITE_LENGTH];
        uint8_t data_len = 0;
        char* data_str;
        
        while ((data_str = strtok(NULL, " \t\r\n")) != NULL && data_len < MAX_WRITE_LENGTH) {
            tx_buffer[data_len++] = (uint8_t)strtol(data_str, NULL, 0);
        }
        
        if (data_len == 0) {
            strcpy(message, "No data to write. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        
        uint8_t cs_pin = (uint8_t)strtol(cs_pin_str, NULL, 0);
        
        GPIO_WritePin(cs_pin, GPIO_PIN_RESET);
        
        HAL_StatusTypeDef status = SPI_Transmit(tx_buffer, data_len);
        
        GPIO_WritePin(cs_pin, GPIO_PIN_SET);
        
        if (status == HAL_OK) {
            snprintf(message, sizeof(message), "Successfully wrote %d bytes to SPI with CS pin %d\r\n", 
                     data_len, cs_pin);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
        } else {
            snprintf(message, sizeof(message), "Error: SPI write failed with status %d\r\n", status);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
        }
        break;
    }

    case CMD_SPI_READ:
    {
        char* cs_pin_str = strtok(NULL, " \t\r\n");
        if (cs_pin_str == NULL) {
            strcpy(message, "CS pin is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        
        char* size_str = strtok(NULL, " \t\r\n");
        if (size_str == NULL) {
            strcpy(message, "Bytes to read is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        
        uint8_t size = (uint8_t)strtol(size_str, NULL, 0);
        if (size > MAX_READ_LENGTH) {
            snprintf(message, sizeof(message), "Warning: Read length limited to %d bytes\r\n", MAX_READ_LENGTH);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            size = MAX_READ_LENGTH;
            rearm_uart();
        }
        
        uint8_t cs_pin = (uint8_t)strtol(cs_pin_str, NULL, 0);
        uint8_t rx_buffer[MAX_READ_LENGTH];
        
        
        GPIO_WritePin(cs_pin, GPIO_PIN_RESET);
        
        HAL_StatusTypeDef status = SPI_Receive(rx_buffer, size);
        
        GPIO_WritePin(cs_pin, GPIO_PIN_SET);
        
        if (status == HAL_OK) {
            snprintf(message, sizeof(message), "Read %d bytes from SPI with CS pin %d:\r\n", size, cs_pin);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            
            // Print data in rows of 8 bytes
            for (int i = 0; i < size; i++) {
                snprintf(message, sizeof(message), "0x%02X ", rx_buffer[i]);
                HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
                
                // Add a newline after every 8 bytes
                if ((i + 1) % 8 == 0) {
                    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
                }
            }
            
            // Add final newline if not already added
            if (size % 8 != 0) {
                HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
            }
            rearm_uart();
        } else {
            snprintf(message, sizeof(message), "Error: SPI read failed with status %d\r\n", status);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
        }
        break;
    }
    
    case CMD_SPI_TRANSFER:
    {
        char* cs_pin_str = strtok(NULL, " \t\r\n");
        if (cs_pin_str == NULL) {
            strcpy(message, "CS pin is empty. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        
        uint8_t tx_buffer[MAX_WRITE_LENGTH];
        uint8_t rx_buffer[MAX_READ_LENGTH];
        uint8_t data_len = 0;
        char* data_str;
        
        while ((data_str = strtok(NULL, " \t\r\n")) != NULL && data_len < MAX_READ_LENGTH) {
            tx_buffer[data_len++] = (uint8_t)strtol(data_str, NULL, 0);
        }
        
        if (data_len == 0) {
            strcpy(message, "No data to transfer. Please try again.\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
            return;
        }
        
        uint8_t cs_pin = (uint8_t)strtol(cs_pin_str, NULL, 0);
        
        GPIO_WritePin(cs_pin, GPIO_PIN_RESET);
        
        HAL_StatusTypeDef status = SPI_TransmitReceive(tx_buffer, rx_buffer, data_len, 1000);
        
        GPIO_WritePin(cs_pin, GPIO_PIN_SET);
        
        if (status == HAL_OK) {
            snprintf(message, sizeof(message), "SPI transfer with CS pin %d:\r\n", cs_pin);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            
            HAL_UART_Transmit(&huart2, (uint8_t*)"Sent: ", 6, HAL_MAX_DELAY);
            for (int i = 0; i < data_len; i++) {
                snprintf(message, sizeof(message), "0x%02X ", tx_buffer[i]);
                HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            }
            HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
            
            HAL_UART_Transmit(&huart2, (uint8_t*)"Received: ", 10, HAL_MAX_DELAY);
            for (int i = 0; i < data_len; i++) {
                snprintf(message, sizeof(message), "0x%02X ", rx_buffer[i]);
                HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            }
            HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
            
            rearm_uart();
        } else {
            snprintf(message, sizeof(message), "Error: SPI transfer failed with status %d\r\n", status);
            HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
            rearm_uart();
        }
        break;
    }

    // Display help menu
    case CMD_HELP:
        print_menu();
        break;

    
    
    // Invalid command
    default:
        strcpy(message, "Invalid command\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
        rearm_uart();
    }
}