typedef enum {
    CMD_I2C_SCAN = 1,
    CMD_I2C_WRITE = 2,
    CMD_I2C_READ = 3,
    CMD_I2C_MEM_READ = 4,
    CMD_I2C_MEM_WRITE = 5,
    CMD_I2C_SPEED = 6,
    CMD_I2C_RESET = 7,
    CMD_SPI_READ = 8,
    CMD_SPI_WRITE = 9,
    CMD_SPI_TRANSFER = 10,
    CMD_HELP = 11
} commands_t;