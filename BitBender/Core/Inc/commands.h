typedef enum{
    CMD_I2C_SCAN = 1,
    CMD_I2C_WRITE = 2,
    CMD_I2C_READ = 3,
    CMD_I2C_MEM_READ = 4
}commands_t;

typedef struct{
    uint8_t commandReady: 0;
    uint8_t setAddress: 0;
    uint8_t setData: 0;
} control_flag_t;

typedef struct{
    uint8_t address;
    uint8_t data;
}command_parmas_t;