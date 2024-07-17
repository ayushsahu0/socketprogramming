#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "ble.h"
#include "wifi.h"
#include "esp_err.h"

#define BOOT_BUTTON_PIN GPIO_NUM_0 // Change this to the actual GPIO number connected to the boot button
#define MAINTAG "MAIN"

char buffer[128];

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

// Function to generate a random number and convert it to a string
// static void random_no_Generator(char *payload, size_t payload_size) {
//     if (payload == NULL || payload_size == 0) {
//         // Log an error if buffer is NULL or size is zero
//         ESP_LOGE(MAINTAG, "Invalid buffer or size");
//         return;
//     }

//     // Generate a random number
//     uint32_t random_no = esp_random();

//     // Extract the last 2 digits of the random number
//     uint8_t rem = random_no % 100;

//     // Log the random number
//     ESP_LOGI(MAINTAG, "The Random no is: %u", rem);

//     // Format the random number as a string
//     int result = snprintf(payload, payload_size, "%02u", rem);

//     if (result < 0) {
//         // Log an error if snprintf fails
//         ESP_LOGE(MAINTAG, "Failed to format the random number");
//     } else if ((size_t)result >= payload_size) {
//         // Log a warning if the result was truncated
//         ESP_LOGW(MAINTAG, "Payload truncated: buffer too small");
//     }
// }

void app_main()
{
    delay();
    //random_no_Generator(buffer,sizeof(buffer));
    ble_init(buffer,sizeof(buffer));
    udp_init();
    udp_server_task();

    // Initialize BLE (Bluetooth Low Energy)
    // esp_err_t ble_result = ble_init(buffer,sizeof(buffer));
    // if (ble_result == ESP_OK)
    // {
    //     ESP_LOGI(TAG, "BLE initialization successful.");

    //     // Initialize UDP
    //     esp_err_t udp_result = udp_init();
    //     if (udp_result == ESP_OK)
    //     {
    //         ESP_LOGI(TAG, "UDP initialization successful.");

    //         // Create a FreeRTOS task to handle UDP server operations
    //         udp_server_task(buffer,sizeof(buffer));
    //         xTaskCreate(udp_server_task, "udp_server_task", 4096, NULL, 5, NULL);
    //         ESP_LOGI(TAG, "UDP server task started.");
    //     }
    //     else
    //     {
    //         ESP_LOGE(TAG, "UDP initialization failed.");
    //     }
    // }
    // else
    // {
    //     ESP_LOGE(TAG, "BLE initialization failed.");
    // }

   return ;
}
