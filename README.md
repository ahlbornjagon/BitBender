![](BitBender/images/bitbender.jpg)


## What is it?
BitBender is a program written for the STM32 (soon to be followed by targets) that allows for interactive control of devices on various protocols. 

Current protocols supported are: 
- I2C
- SPI

## Current commands supported:
 
| Command                |     Function                  |
| -----------------------| ----------------------------- |
|       i2cscan          | Scan for devices              |
|       i2cread          | Read from device              |
|       i2creadmem       | Read from device memory       |
|       i2cwrite         | Write to device               |
|       i2cwritemem      | Write to device memory        |
|       i2cspeed         | Set communication speed       |
|       i2creset         | Reset I2C Bus                 |
|       spiread          | Read data from spi device     |
|       spiwrite         | Write data to spi device      |
|       spitransfer      | Read and write to spi device  |
|       help             | Display command menu          |


## Example usage:

Perform an i2c scan of devices on the i2c bus.
```
i2cscan
```

Read 16 bytes from an i2c device at address 0x44.
```
i2cread 0x44 16
```

Write data to a spi device (**TODO: CHIP SELECT IS CURRENTLY HARDCODED**)
```
spiwrite 0x02 0x00 0x10 0x00
```

