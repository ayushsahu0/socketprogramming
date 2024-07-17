/* UDP Server */
#ifndef WIFI_H
#define WIFI_H

#include <unistd.h>
#include <string.h>
#include <sys/param.h>
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
#include <esp_wifi.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

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

#define TAG  "UDP_Server"

static void udp_server_task()
{
    // Buffer to hold received data
    char rx_buffer[128];

    // Variables for address family and IP protocol
    int addr_family = 0;
    int ip_protocol = 0;

    // Port where the client will listen
    int client_port = 3333;

#if defined(CONFIG_EXAMPLE_IPV4)
    // Configuration for IPv4
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR); // Destination IP address
    dest_addr.sin_family = AF_INET; // Address family for IPv4
    dest_addr.sin_port = htons(client_port); // Port number in network byte order
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_EXAMPLE_IPV6)
    // Configuration for IPv6
    struct sockaddr_in6 dest_addr = { 0 };
    inet6_aton(HOST_IP_ADDR, &dest_addr.sin6_addr); // Destination IP address for IPv6
    dest_addr.sin6_family = AF_INET6; // Address family for IPv6
    dest_addr.sin6_port = htons(client_port); // Port number in network byte order
    dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE); // Scope ID
    addr_family = AF_INET6;
    ip_protocol = IPPROTO_IPV6;
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
    // Configuration for destination address from standard input
    struct sockaddr_storage dest_addr = { 0 };
    ESP_ERROR_CHECK(get_addr_from_stdin(client_port, SOCK_DGRAM, &ip_protocol, &addr_family, &dest_addr));
#endif

    // Create a UDP socket
    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL); // Delete the task if socket creation fails
    }

    // Bind the socket to the local port
    struct sockaddr_in bind_addr;
    bind_addr.sin_family = AF_INET; // IPv4 address family
    bind_addr.sin_addr.s_addr = INADDR_ANY; // Bind to any local IP address
    bind_addr.sin_port = htons(client_port); // Port number in network byte order

    if (bind(sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        close(sock); // Close the socket on bind failure
        vTaskDelete(NULL); // Delete the task
    }

    // Set timeout for receiving data
    struct timeval timeout;
    timeout.tv_sec = 10; // Timeout in seconds
    timeout.tv_usec = 0; // Timeout in microseconds
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    ESP_LOGI(TAG, "Socket created and bound to port %d, waiting for data...", client_port);

    // Main loop to receive data
    while (1) {
        struct sockaddr_storage source_addr; // Storage for source address (IPv4 or IPv6)
        socklen_t socklen = sizeof(source_addr);
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

        // Error occurred during receiving
        if (len < 0) {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            if (errno == ENETUNREACH || errno == EHOSTUNREACH) {
                ESP_LOGE(TAG, "Network is unreachable or no route to host.");
            }
        } 
        // Data received successfully
        else {
            rx_buffer[len] = 0; // Null-terminate the received data

            // Log sender's IP address and received data
            if (source_addr.ss_family == AF_INET) {
                char addr_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(((struct sockaddr_in *)&source_addr)->sin_addr), addr_str, sizeof(addr_str));
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);
            } else if (source_addr.ss_family == AF_INET6) {
                char addr_str[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)&source_addr)->sin6_addr), addr_str, sizeof(addr_str));
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
            }

            // Define the acknowledgment message
            const char *ack_message = "Ack_msg";

            // Send acknowledgment message back to the client
            int err = sendto(sock, ack_message, strlen(ack_message), 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            } else {
                ESP_LOGI(TAG, "Acknowledgment message sent: %s", ack_message);
            }

            // Exit the loop after sending acknowledgment
            break;
        }

        // Delay before the next receive attempt
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    // Shutdown and close the socket
    if (sock != -1) {
        ESP_LOGI(TAG, "Shutting down socket and closing...");
        shutdown(sock, 0); // Shutdown the socket
        close(sock); // Close the socket
    }

    // Disconnect from Wi-Fi and clean up resources
    ESP_LOGI(TAG, "Disconnecting from Wi-Fi...");
    esp_wifi_disconnect();
    esp_wifi_stop();
    ESP_LOGI(TAG, "Wi-Fi driver stopped");
    esp_wifi_deinit();
    ESP_LOGI(TAG, "Wi-Fi driver deinitialized");

    // Clean up the task
    vTaskDelete(NULL);
    ESP_LOGI(TAG, "Task deleted");
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
    
    //xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 1, NULL); // Create the UDP server task
}

#endif // WIFI_H
