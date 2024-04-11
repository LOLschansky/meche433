
#include <stdio.h>

#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

//Device address
#define ADDR _u(0b0100000)

// Hardware registers
#define REG_IODIR _u(0x00)
#define REG_GPIO _u(0x09)
#define REG_OLAT _u(0x0A)


void chip_init() {
    // use the "handheld device dynamic" optimal setting (see datasheet)
    uint8_t buf[2];

    // set GP7 as output, rest as input
    buf[0] = REG_IODIR;
    buf[1] = 0b01111111;
    i2c_write_blocking(i2c_default, ADDR, buf, 2, false);
}

void set(char v) {
    uint8_t buf[2];

        // send register number followed by its corresponding value
    buf[0] = REG_OLAT;
    buf[1] = v << 7;
    i2c_write_blocking(i2c_default, ADDR, buf, 2, false);
}

int read() {
    uint8_t buf[1];
    uint8_t reg = REG_GPIO;

    i2c_write_blocking(i2c_default, ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_default, ADDR, buf, 1, false);

    if (buf[0]&0b1 == 0b1){
        return 1;
    }
    else{
        return  0;
    }
}




int main() {
    stdio_init_all();

    // I2C is "open drain", pull ups to keep signal high when no data is being sent
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    //Pico heartbeat
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);


    // initializie the chip
    chip_init(); // gp7 is output, gp0 is input

  
    //sleep_ms(250); // sleep so that data polling and register update don't collide
    while (1) {
        //Pico heartbeat
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
        
        //set(1);

        
        if (read()){
            set(1);
        }
        else{
            set(0);
        }
        




        //manual testing
        /*
        uint8_t buf[2];

        // send register number followed by its corresponding value
        buf[0] = REG_OLAT;
        buf[1] = 0b10000000;
        i2c_write_blocking(i2c_default, ADDR, buf, 2, false);
        */
        
        
    }
}



