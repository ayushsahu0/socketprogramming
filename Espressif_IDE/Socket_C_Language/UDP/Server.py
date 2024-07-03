// Python UDP Server Code for Firstly recieving a message from the Client1 and sending the same message with a random no attached to it to the second client

import socket
import random

# Server's IP address and port
server_ip = '172.16.8.238'
server_port = 9999

# Client's IP address and port for forwarding
client_ip = '172.16.8.87'   // client ip is shown in the terminal of the client when wifi is connected ipV4 address is shown there itself
client_port = 3333

# Create a UDP socket for receiving messages
recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to the server address and port
recv_sock.bind((server_ip, server_port))

print(f"Server listening on {server_ip}:{server_port}")

while True:
    # Wait for a message from the first client
    data, address = recv_sock.recvfrom(4096)

    # Check if the message is from the expected client
    if address[0] == '172.16.8.145':                                  //Address of the first client
        print(f"Received message from {address}: {data.decode()}")

        # Send an acknowledgment back to the first client
        ack_response = "Acknowledgment: Message received"
        recv_sock.sendto(ack_response.encode(), address)

        # Generate a random number
        random_number = random.randint(1, 1000)

        # Create a new UDP socket for sending the message to the second client
        send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # Send the message along with the random number to the second client
        message = f"{data.decode()} with random number: {random_number}"
        send_sock.sendto(message.encode(), (client_ip, client_port))
        print(f"Sent message to {client_ip}:{client_port}")

        # Close the sending socket
        send_sock.close()
    else:
        print(f"Received message from unexpected client {address}: {data.decode()}")
