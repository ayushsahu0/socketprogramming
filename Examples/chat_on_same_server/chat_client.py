# multiple client chat with each other through the same server.

# client.py
import socket
import threading

# Client configuration
SERVER_IP = '127.0.0.1'
SERVER_PORT = 5005
BUFFER_SIZE = 1024

# Function to handle receiving messages from the server
def receive_messages(client_socket):
    while True:
        try:
            message = client_socket.recv(BUFFER_SIZE).decode('utf-8')
            if not message:
                break
            print(message)
        except:
            break

# Set up client
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((SERVER_IP, SERVER_PORT))

# Start thread to listen for incoming messages
receive_thread = threading.Thread(target=receive_messages, args=(client_socket,))
receive_thread.start()

# Main loop to send messages
print("Type messages to send to the other client. Type 'exit' to quit.")
while True:
    message = input()
    if message.lower() == 'exit':
        break
    client_socket.sendall(message.encode('utf-8'))

client_socket.close()
