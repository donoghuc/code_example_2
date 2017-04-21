# Another code example
This is another school project. I waited until class was over to upload and tried to make the repo not obviously searchable. 
# Intro
This README describes the use of the FTP server/client implemented according to the 
project 2 specifications. The basic idea is that the server (implemented in C) waits for 
a connection from client (implemented in python3) on a communication socket. The client and 
server arrange a data connection socket. The idea is that communication happens over the 
communicatiion socket and the data transfer happens over the data socket. 
# Installation

### ft_server specifics
1: ft_server.c

2: c_server_functions.c

3: c_server_functions.h

4: makefile

### ft_client specifics
5: ft_client.py

6: socket_class.py

in order to test the file transfer i run the client in a different folder (in different console)
than server. So i would suggest making a folder for the ft_server to run and a folder for the 
ft_client to run. Once you have desired folders move the ft_server specifics and ft_client specifics
to their appropriate newly created folders. 

### Compiling ft_server
compile by simply running 
```
>>make
```
(assuming you have ft_server.c, c_server_functions.c, c_server_functions.h, and makefile in same directory)

### Starting ft_server
run with
```
>>./ft_server [PORT NUMBER]
```
the [PORT NUMBER] will be the desired communication port. 
note the server name and port number when successful startup occurs 

### Starting ft_client
make sure a copy of ft_client.py and socket_class.py are in a client directory (separate from ft_server)
and that you have a ft_server running (noting the host and portnum)

### For List directory request
```
>>python3 ft_client.py [SERVER NAME] [SERVER PORT] -l [DATA PORT]
```
[SERVER NAME] : note this from when you started ft_server

[SERVER PORT] : note this from when you started ft_server

-l : indicate you want a list dir

[DATA PORT] : come up with a data port you want for the data socket

### For get file request:
```
>>python3 ft_client.py [SERVER NAME] [SERVER PORT] -g [FILE NAME] [DATA PORT]
```
[SERVER NAME] : note this from when you started ft_server

[SERVER PORT] : note this from when you started ft_server

-g : indicate you want a file

[FILE NAME] : the file you want

[DATA PORT] : come up with a data port you want for the data socket

### Notes
-if a file allready exists on the client side you will be asked if you want to overwrite or abort

-if a file does not exist on the server you will be notified that it is impossible to recieve
