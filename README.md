# Chat Server
implement an simple event-driven chat server, the program combines work with a variety of TCP features such as:

🔹 select function (In order to save on the use of a thread pool for clients)

🔹 sockets

## Technologies
👉 C programming language

## Supported operating systems
💻 Linux

💻 Unix
 
## Implementation Remarks

🔸 I have added to the program 3 structures that will help me perform the task in a simpler way:

 1) conn_pool_t - Data structure to keep track of active client connections.

 2) msg_t - Data structure to keep track of messages. Each message object holds one.

 3) conn_t - Data structure to keep track of client connection state.

🔸 I have added to the program 5 main methodes that will help me perform the task in a simpler way:

 1) int init_pool - Init the conn_pool_t structure.

 2) int add_conn - Add connection when new client connects the server.

 3) int remove_conn - Remove connection when a client closes connection, or clean memory if server stops.

 4) int add_msg - Add msg to the queues of all connections (except of the origin).

 5) int write_to_client - Write msg to client.

🔸 I have added to the program 9 auxiliary methods that will help me perform the task in a simpler way:

 1) void input_test - Before running the program we will check if the input of the program was indeed transmitted as required.

 2) int create_welcome_socket - create welcome socket,additionally set that all sockets that are created from this welcome socket will be non-blocking.

 3) int create_client_socket create socket to the client.

 4) void intHandler - Once the program receives a SIGINT signal it will simply exit the loop where the chat is running.

 5) int init_pool - Init the conn_pool_t structure.

 6) int add_conn - Add connection when new client connects the server. 

 7) int remove_conn - Remove connection when a client closes connection, or clean memory if server stops.

 8) int add_msg - Add msg to the queues of all connections (except of the origin).

 9) int write_to_client - Write msg to client.

🔸 We will listen to the ports only within the range 1024-10000, this is the range of ports that are commonly used to communicate under 'TCP' protocol.

🔸 It is usually customary to set that they can wait in line for the service so welcome socket no more than 5 clients.

## how to run

The recommendation is by using the telnet command With several terminals.

On your main terminal :

```
~$: gcc chatServer.c -o <program-name> 
~$: ./<program-name> <port-number>
```

In the rest of the terminals you decide to open : 

```
~$: telnet localhost <port-number>
```

And hence only what is left is to write from one terminal to another.
## for any questions

```
if(haveAnyQuestions === true){
    let yourName = ".......", question = ".......";
    sendEmailToMe(yourName,question,odedatias8115@gmail.com);
}
```
