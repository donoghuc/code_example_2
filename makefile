default: ft_server

c_server_functions.o: c_server_functions.c
	gcc -c c_server_functions.c

ft_server.o: ft_server.c
	gcc -c ft_server.c

ft_server: c_server_functions.o ft_server.o
	gcc -o ft_server c_server_functions.o ft_server.o

clean:
	rm c_server_functions.o ft_server.o