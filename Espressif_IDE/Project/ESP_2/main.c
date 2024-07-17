#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "ble.h"
#include "wifi.h"
#include <inttypes.h>
#include <stdint.h>

#define BOOT_BUTTON_PIN GPIO_NUM_0 // Change this to the actual GPIO number connected to the boot button

#define MAX_MESSAGE_SIZE 256  // Define maximum size for received_message

// Global variables to store received message and its length
// Define the global variable


static void delay (){
    // Initialize the GPIO pin as input with a pull-up resistor
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BOOT_BUTTON_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    printf("Waiting for boot button press...\n");

    // Wait indefinitely until the boot button is pressed
    while (gpio_get_level(BOOT_BUTTON_PIN) == 1) {
        // Button is not pressed (assuming HIGH means not pressed)
        // Add a small delay to reduce CPU usage
        vTaskDelay(100 / portTICK_PERIOD_MS); // Delay for 100 milliseconds
    }

    printf("Boot button pressed! Continuing with the program...\n");
    return;
}

static void udp(){
   udp_init();
   udp_client_task();
}

void app_main()
{ 
    delay();
    ble_init();
    udp();
    
   return ;
}
