README

# Chat Program

## Overview

This program is a simple chat application that allows users to connect with each other over TCP/IP connections. It provides a user-friendly command-line interface for managing connections and exchanging messages.

## Running the Program

To run the program, execute the following command in the terminal:

```
./chat <port_number>
```

Replace `<port_number>` with the port on which you want the program to listen for incoming connections.

## Functionality

Upon launching, the program acts as a UNIX shell and provides the following command options:

1. **help**:  Display information about the available user interface options or command manual.

2. **myip**: Display the IP address of this process.

3. **myport**: Display the port on which this process is listening for incoming connections.

4. **connect <destination> <port_no>**: Establish a new TCP connection to the specified destination at the specified port.

5. **list**: Display a numbered list of all the connections this process is part of.

6. **terminate <connection_id>**: Terminate the connection with connection id from connection list.

7. **send <connection_id>** **<message>**: Send a message to the connection with connection id from connection list.

8. **exit**: Close all connections and terminate this process.

