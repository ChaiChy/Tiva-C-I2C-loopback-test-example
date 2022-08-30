#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

#include "inc/tm4c123gh6pm.h"
//#include ""


//init I2C module 2
static void init_i2c2_master(void)
{
    //enable module
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);

    //reset module
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C2);

    //enable GPIO peripheral that contains I2C 2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // Configure the pin muxing for I2C2 functions on port E4 and E5.
    GPIOPinConfigure(GPIO_PCTL_PE4_I2C2SCL);
    GPIOPinConfigure(GPIO_PCTL_PE5_I2C2SDA);

    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTE_BASE, GPIO_PIN_5);
    GPIOPinTypeI2C(GPIO_PORTE_BASE, GPIO_PIN_6);

    // Enable and initialize the I2C2 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    I2CMasterInitExpClk(I2C2_BASE, SysCtlClockGet(), false);

    //clear IC2 FIFO
    HWREG(I2C2_BASE + I2C_O_FIFOCTL) = 80008000; //wtf is this

}

static void init_i2c1_slave(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);

    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PCTL_PA6_I2C1SCL);
    GPIOPinConfigure(GPIO_PCTL_PA7_I2C1SDA);

    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

    // Enable and initialize the I2C1 slave module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    I2CSlaveInit(I2C1_BASE, 0x11);
    I2CSlaveAddressSet(I2C1_BASE, 0, 0x11);
    I2CSlaveAddressSet(I2C1_BASE, 1, 0x10);
    //clear IC2 FIFO
    HWREG(I2C1_BASE + I2C_O_FIFOCTL) = 80008000;

}


/*
//sends an I2C command to the specified slave
void I2CSend(uint8_t slave_addr, uint8_t num_of_args, uint32_t data_to_be_transferred)
{
    // Tell the master module what address it will place on the bus when
    // communicating with the slave.
    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, false);


    //put data to be sent into FIFO
    I2CMasterDataPut(I2C2_BASE, va_arg(vargs, uint32_t)); //value to be inserted

    //if there is only one argument
    if(num_of_args == 1)
    {
        //Initiate send of data from the MCU
        I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_SEND);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C2_BASE));

        //"close" variable argument list
        va_end(vargs);
    }

    //otherwise, we start transmission of multiple bytes on the
    //I2C bus
    else
    {
        I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_START);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C2_BASE));

        //send num_of_args-2 pieces of data, using the
        //BURST_SEND_CONT command of the I2C module
        for(uint8_t i = 1; i < (num_of_args - 1); i++)
        {
            //put next piece of data into I2C FIFO
            I2CMasterDataPut(I2C2_BASE, va_arg(vargs, uint32_t));
            //send next data that was just placed into FIFO
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

            // Wait until MCU is done transferring.
            while(I2CMasterBusy(I2C0_BASE));
        }

        //put last piece of data into I2C FIFO
        I2CMasterDataPut(I2C0_BASE, va_arg(vargs, uint32_t));
        //send next data that was just placed into FIFO
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C0_BASE));

        //"close" variable args list
        va_end(vargs);
    }
}
*/




int main(void)
{
    clock_setup();

    init_i2c2_master();
    init_i2c1_slave();

    uint8_t data = 0xA5; // data to be incremented
    uint8_t final_value = 0x00;

    //sending out slv address to the bus
    //false == writing to slave
    I2CMasterSlaveAddrSet(I2C2_BASE, 0x11, false);
    //sending data to bus
    I2CMasterDataPut(I2C2_BASE, data);

    //initiate send of data from the MCU
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    //wait for data transm to finsih
    while (I2C_MASTER_BUSY(I2C2_BASE));

    //slavery time
    final_value += I2CSlaveDataGet(I2C1_BASE);

    return 0;
}
