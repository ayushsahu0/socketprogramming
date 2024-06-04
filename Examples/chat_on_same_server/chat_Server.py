# multiple client chat with each other through the same server.

# server.py
import socket
import threading

# Server configuration
SERVER_IP = '127.0.0.1'
SERVER_PORT = 5005
BUFFER_SIZE = 1024

# Store connected clients
clients = []

# Function to handle client communication
def handle_client(client_socket, client_address):
    print(f"New connection: {client_address}")
    while True:
        try:
            message = client_socket.recv(BUFFER_SIZE).decode('utf-8')
            if not message:
                break
            print(f"Received from {client_address}: {message}")

            # Relay the message to the other clients
            for client in clients:
                if client != client_socket:    #This if statement ensures that the server does not send the message back to the client that sent it.
                    client.sendall(f"Message from {client_address}: {message}".encode('utf-8'))
        except:
            break
    print(f"Connection closed: {client_address}")
    clients.remove(client_socket)
    client_socket.close()

# Set up server
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((SERVER_IP, SERVER_PORT))
server_socket.listen(2)
print(f"Server listening on {SERVER_IP}:{SERVER_PORT}")

# Main loop to accept and handle clients
while True:
    client_socket, client_address = server_socket.accept()
    clients.append(client_socket)
    client_handler = threading.Thread(target=handle_client, args=(client_socket, client_address))
    client_handler.start()
