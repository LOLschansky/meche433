/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#include <stdio.h>
#include <string.h>
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/gpio.h"

void drawChar(int x, int y, char letter);
void drawMessage(int x, int y, char* message);

char message[50];
/// \tag::uart_advanced[]

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

static int chars_rxed = 0;
int x = 0;

// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        
        if (ch == 13){
            //sprintf(&message[x],"%d", 0);
            ssd1306_clear();
            sprintf(&message[x],"0");
            drawMessage(0,16, message);
            ssd1306_update();
            x = 0;
        }
        else{
        sprintf(&message[x],"%c", ch);
        x = x + 1;
        }


        //sprintf(message,"hello");

        //drawChar(32,12,message[0]);
        // drawMessage(32,12, message);
        

        /*
        if (ch == 13){
            x = 0;
            ssd1306_clear();
            ssd1306_update();

        }
        else{
            drawChar(x, 0, ch);
		    ssd1306_update();
            x = x + 5;
        }
        */

        // Can we send it back?
        if (uart_is_writable(UART_ID)) {
            // Change it slightly first!
            //ch++;
            uart_putc(UART_ID, ch);
        }
        chars_rxed++;
    }
}

int main() {
    stdio_init_all();

    // I2C is "open drain", pull ups to keep signal high when no data is being sent
    i2c_init(i2c_default, 100 * 20000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    //Pico heartbeat
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    //Dispaly setup
    ssd1306_setup();
    
    // Set up our UART with a basic baud rate.
    uart_init(UART_ID, 2400);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    // OK, all set up.
    // Lets send a basic string out, and then run a loop and wait for RX interrupts
    // The handler will count them, but also reflect the incoming data back with a slight change!
    uart_puts(UART_ID, "\nHello, uart interrupts\n");



    while (1){
        gpio_put(LED_PIN, 1);
        //ssd1306_drawPixel(64, 16, 1);
        //ssd1306_update();
        sleep_ms(200);
        gpio_put(LED_PIN, 0);
        sleep_ms(200);
    }
}

void drawChar(int x, int y, char letter){
    int i, j;
    for (i = 0; i<5; i++){
        char c = ASCII[letter-32][i];
        for (j = 0; j<8; j++){
            char bit = (c>>j)&0b1;
            if (bit == 0b1){
                ssd1306_drawPixel(x+i, y+j,1);
            }
            else{
                ssd1306_drawPixel(x+i,y+j,0);
            }
        }
    }
}

void drawMessage(int x, int y, char* message){
    int i = 0;
    while (message[i+1]>0){
     drawChar(x+i*5,y,message[i]);
     i++;
    }
}
/// \end:uart_advanced[]
