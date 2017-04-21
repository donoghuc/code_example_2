/* 
Cas Donoghue
CS372 
Project 2: FTP server
use: ./ft_server [PORT_NUMBER]
desc: simple file server that can send directory list or file requested over TCP
using a control connection and a data connection. 
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>

// this has all the implementations for my functions
#include "c_server_functions.h"

//left error message and sigint handler in main bc i fealt it is informative
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
// calling exit should clean up (including closing any open sockets)
void sigint_handler(int sig){
	printf("\nServer closed with SIGINT.\n");
	exit(0);
}

int main(int argc, char *argv[]) {

	/*###################################
	###      server initialization    ###
	###################################*/
	int com_srv_sock_fd, new_sock_fd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    
    com_srv_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (com_srv_sock_fd < 0) {
        error("ERROR opening socket");
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(com_srv_sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }
    // added a spec of hostname to make it easier for client to call it out (ie who memorizes the flip address?)
    char hostname[1024];
    gethostname(hostname, 1024);
    printf("Server open on %s : %d\n", hostname, portno);
    // catch the sigint and close down the server
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

	/*###################################
	###      loop while serving      ###
	###################################*/

    while(1) {
    	listen(com_srv_sock_fd,1);
		clilen = sizeof(cli_addr);
    	new_sock_fd = accept(com_srv_sock_fd, (struct sockaddr *) &cli_addr, &clilen);
    	if(new_sock_fd < 0) {
    		printf("ERROR on accept\n");
    		break;
    	}
    	// this lets the server know that the client is connected and is ready for action
    	char *connection_ok_msg;
    	connection_ok_msg = receive_message(new_sock_fd);
    	if(strcmp(connection_ok_msg, "ok") !=0) {
    		printf("connection error from client\n");
    		close(new_sock_fd);
    		free(connection_ok_msg);
    		break;
    	}
    	free(connection_ok_msg);
    	//get the service type and details
    	char *data_connection_info;
    	data_connection_info = receive_message(new_sock_fd);
    	//get request and data connection information
    	int buffer_size = 96;
		char data_server_name[buffer_size];
		char data_server_port[buffer_size];
		char service_requested[buffer_size];
		char file_requested[buffer_size];
		parse_service_request(data_connection_info, data_server_name, data_server_port, service_requested, file_requested);
		printf("Connection from %s\n", data_server_name);
		//set up data connection (as client to server running on ft_client)
		int data_sock_send, send_to_portno;
		struct sockaddr_in server_name_data;
		struct hostent *server_name;
		server_name = gethostbyname(data_server_name);
		send_to_portno = atoi(data_server_port);
		data_sock_send = socket(AF_INET, SOCK_STREAM, 0);
		if(data_sock_send < 0){
			printf("ERROR opening socket\n");
			send_message(new_sock_fd, "2xxno");
			close(new_sock_fd);
			break;
		}
		if(server_name == NULL) {
			printf("ERROR no such host\n");
			send_message(new_sock_fd, "2xxno");
			close(new_sock_fd);
			break;
		}
		bzero((char*) &server_name_data, sizeof(server_name_data));
		server_name_data.sin_family = AF_INET;
		bcopy((char*)server_name->h_addr,(char*)&server_name_data.sin_addr.s_addr,server_name->h_length);
		server_name_data.sin_port = htons(send_to_portno);
		if(connect(data_sock_send,(struct sockaddr*) &server_name_data,sizeof(server_name_data)) < 0){
			printf("ERROR cannot connect\n");
			send_message(new_sock_fd, "2xxno");
			close(new_sock_fd);
			break;
		}
		// data connection on server side is OK alert client via communication socket
		send_message(new_sock_fd, "2xxok");
		/*################################################## 
		########## LIST requested, deal with it ############
		##################################################*/
		if(strcmp(service_requested,"list") == 0){
			printf("List directory requested on port %s\n", data_server_port);
			printf("Sending directory contents to %s:%s\n", data_server_name, data_server_port);

			int allocation_len = calc_allocation_len();
			char *comma_sep_dir;
			comma_sep_dir = comma_sep_dir_str(allocation_len);
			int bytes_expected = strlen(comma_sep_dir);
			char *len_expected_msg;
			len_expected_msg = format_expected_len_msg(bytes_expected);
			//send expected size over communication socket and data over data socket
			send_message(new_sock_fd, len_expected_msg);
			send_message(data_sock_send, comma_sep_dir);

			free(comma_sep_dir);
			free(len_expected_msg);

		}
		/*################################################## 
		########## GET requested, deal with it ############
		##################################################*/
		else if (strcmp(service_requested, "get") == 0){
			printf("File \"%s\" requested on port %s\n", file_requested, data_server_port);
			//check if file exists, if so send it
			if (check_if_file_exists(file_requested) == 0){
				printf("Sending \"%s\" to %s:%s\n",file_requested, data_server_name, data_server_port);
				int file_size = calc_file_size(file_requested);
				char* file_len_msg;
				file_len_msg = format_expected_len_msg(file_size);
				//send file size (expecteed length) over communication socket
				send_message(new_sock_fd, file_len_msg);
				free(file_len_msg);
				//send file over data socket
				chunks_for_sending(file_size, file_requested, data_sock_send);
			}
			// file does not exist, print and send error over communication socket
			else{
				printf("File not found. Sending error message to %s:%d\n", data_server_name, portno);
				send_message(new_sock_fd, "14xFILE NOT FOUND");
			}
		}
		//clean up for next loop through server. 
    	free(data_connection_info);
    	close(new_sock_fd);
    	close(data_sock_send);
	    }
	//never get here...
	return 0; 
}



