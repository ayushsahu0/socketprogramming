/*  UDP Client */
#ifndef WIFI_H
#define WIFI_H

#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <esp_wifi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

char received_message[1024]; 

#ifdef CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN
#include "addr_from_stdin.h"
#endif

#if defined(CONFIG_EXAMPLE_IPV4)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#elif defined(CONFIG_EXAMPLE_IPV6)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#else
#define HOST_IP_ADDR ""
#endif

#define PORT CONFIG_EXAMPLE_PORT

#define TAG  "UDP_Client"

static void udp_client_task()
{
    int addr_family = 0;
    int ip_protocol = 0;

#if defined(CONFIG_EXAMPLE_IPV4)
    // Configure destination address for IPv4
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_EXAMPLE_IPV6)
    // Configure destination address for IPv6
    struct sockaddr_in6 dest_addr = { 0 };
    inet6_aton(HOST_IP_ADDR, &dest_addr.sin6_addr);
    dest_addr.sin6_family = AF_INET6;
    dest_addr.sin6_port = htons(PORT);
    dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
    addr_family = AF_INET6;
    ip_protocol = IPPROTO_IPV6;
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
    // Configure destination address from standard input
    struct sockaddr_storage dest_addr = { 0 };
    ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_DGRAM, &ip_protocol, &addr_family, &dest_addr));
#endif

    // Create a socket
    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
    }

    // Set timeout for receiving data
    struct timeval timeout;
    timeout.tv_sec = 10; // Set timeout to 10 seconds
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

    // Define the expected acknowledgment message
    const char *ACK_MSG = "Ack_msg";  // Change this to your actual acknowledgment message
    char buffer[1024];

    while (1) {
        // Send the message to the destination address
        int err = sendto(sock, received_message, strlen(received_message), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait before retrying
            continue;
        }

        ESP_LOGI(TAG, "Message sent: %s", received_message);

        // Wait for acknowledgment
        int recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
        if (recv_len < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Handle the non-blocking case or timeout
                ESP_LOGW(TAG, "No acknowledgment received. Retrying...");
                vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait before retrying
                continue;
            } else {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break; // Exit loop on error
            }
        }

        // Null-terminate the received data
        buffer[recv_len] = 0;

        // Compare the first 7 characters of the received message with ACK_MSG

        if (strncmp(buffer, ACK_MSG, 7) == 0) {
            ESP_LOGI(TAG, "Received acknowledgment: %s", buffer);
            ESP_LOGI(TAG, "Acknowledgment received, exiting task.");
            
            // Clean up the socket and Wi-Fi connection
            close(sock);
            ESP_LOGI(TAG, "Disconnecting from Wi-Fi...");
            esp_wifi_disconnect();
            esp_wifi_stop();
            ESP_LOGI(TAG, "Wi-Fi driver stopped");
            esp_wifi_deinit();
            ESP_LOGI(TAG, "Wi-Fi driver deinitialized");

            // Clean up the task
            vTaskDelete(NULL);
        } else {
            ESP_LOGW(TAG, "Acknowledgment message is not as expected. Received: %s", buffer);
            vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait before retrying
        }
    }

    // Close the socket if we exit the loop
    close(sock);
    ESP_LOGI(TAG, "Socket closed");

    // Disconnect from Wi-Fi and deinitialize the Wi-Fi driver
    ESP_LOGI(TAG, "Disconnecting from Wi-Fi...");
    esp_wifi_disconnect();
    esp_wifi_stop();
    ESP_LOGI(TAG, "Wi-Fi driver stopped");
    esp_wifi_deinit();
    ESP_LOGI(TAG, "Wi-Fi driver deinitialized");

    // Clean up the task
    vTaskDelete(NULL);
}


static void udp_init()
{
    ESP_ERROR_CHECK(nvs_flash_init()); // Initialize NVS flash
    ESP_ERROR_CHECK(esp_netif_init()); // Initialize network interface
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Create default event loop

    ESP_ERROR_CHECK(example_connect()); // Connect to Wi-Fi

    // Log the IP address of the ESP32 device
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif) {
        esp_netif_get_ip_info(netif, &ip_info);
        ESP_LOGI(TAG, "ESP32 IP Address: " IPSTR, IP2STR(&ip_info.ip)); // Log the IP address
    } else {
        ESP_LOGE(TAG, "Failed to get netif handle");
    }

    //xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL); // Create the UDP client task
}

#endif // WIFI_H
