/*Cas Donoghue
CS372 Project2
Function implementations for ft_server.c
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

#include "c_server_functions.h"
/*adapted from Beej's guide. just change send to recv. Simple as that. */
int recvall(int s, char *buf, int *len) {
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;
    while(total < *len) {
        n = recv(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    *len = total; // return number actually sent here
    return n==-1?-1:0; // return -1 on failure, 0 on success
} 
/*this function receives a socket fd and returns the message from sender
it uses my convention of a prefix of 3 chars indicating message length 
for example a message could be 2xxok this indicates message len is 2 and
the returned "string" will be "ok"*/
char* receive_message(int sockfd){
    int prepended_len = 3;
    int extracted_val = 0;
    int n;
    char buffer[4];
    bzero(buffer, 4);
    n = recvall(sockfd, buffer, &prepended_len);
    int i = 0;
    for(i = 0; i < 3; i++){
        if(buffer[i] == 'x'){
            buffer[i] = '\0';
        }
    }
    extracted_val = atoi(buffer);

    char* message_to_receive = malloc(sizeof(*message_to_receive)*(extracted_val+1));
    n = recvall(sockfd, message_to_receive, &extracted_val);
    message_to_receive[extracted_val] = '\0'; 
    return message_to_receive;
}
//Straight out of Beej guide 
int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;
    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    *len = total; // return number actually sent here
    return n==-1?-1:0; // return -1 on failure, 0 on success
} 
//send a formatted message (ie the prefix message len notation)
void send_message(int sockfd, char *message_to_send){
    int numAsStringLen = strlen(message_to_send); 
    int n;
    n = sendall(sockfd, message_to_send, &numAsStringLen);
}
/*This is an ugly function to parse the service request
it takes a comma separated request and bunch of buffer pointers 
(see var names for description and fills them (buffers) in appropriately
 based on service request. Painful bc in python this is basically handled with
 the str.split() method*/
void parse_service_request(char* req_msg, char *data_con_server_name, char *data_con_port_num,
						 char *service_requested, char *file_req){
	
	int len = strlen(req_msg);
	int comma_seps[3] = {-1, -1, -1};
	int num_args = 0;
	//find delimeter (,) index
	int i;
	for (i=0;i<len;i++){
		if(req_msg[i] == ','){
			comma_seps[num_args] = i;
			num_args += 1;
		}
	}
	//GET request parse into appropriate buffers
	if(num_args == 3) {
		int index = 0; 
		for(i=0;i<comma_seps[0];i++){
			data_con_server_name[index] = req_msg[i];
			index += 1; 
		}
		data_con_server_name[index] = '\0';
		index = 0; 
		for(i=comma_seps[0]+1;i<comma_seps[1];i++){
			data_con_port_num[index] = req_msg[i];
			index += 1; 
		}
		data_con_port_num[index] = '\0';
		index = 0; 
		for(i=comma_seps[1]+1;i<comma_seps[2];i++){
			service_requested[index] = req_msg[i];
			index += 1; 
		}
		service_requested[index] = '\0';
		index = 0; 
		for(i=comma_seps[2]+1;i<len;i++){
			file_req[index] = req_msg[i];
			index += 1; 
		}
		file_req[index] = '\0'; 
	}
	//LIST request parse into appropriate buffers
	else if(num_args == 2) {
		int index = 0; 
		for(i=0;i<comma_seps[0];i++){
			data_con_server_name[index] = req_msg[i];
			index += 1; 
		}
		data_con_server_name[index] = '\0';
		index = 0; 
		for(i=comma_seps[0]+1;i<comma_seps[1];i++){
			data_con_port_num[index] = req_msg[i];
			index += 1; 
		}
		data_con_port_num[index] = '\0';
		index = 0; 
		for(i=comma_seps[1]+1;i<len;i++){
			service_requested[index] = req_msg[i];
			index += 1; 
		}
		service_requested[index] = '\0';

	}
}
/* this just finds out how much data to allocate for the comma separated direcotry listing
so for example if there were two files file1 file2 allo len is 5+5+2 (5 for chars in each
file name 1 char for a comma and one for nul term) it returns an int of allocation length*/
int calc_allocation_len() {
	DIR *dp;
	struct dirent *ep;
	int num_files = 0;
	int len_files = 0;
	int allocation_len = 0; 

	dp = opendir("./");
	if(dp != NULL) {
		while (ep = readdir(dp)) {

			if ((strcmp(ep->d_name, ".") != 0) && (strcmp(ep->d_name, "..") != 0)){
				num_files += 1; 
				len_files += strlen(ep->d_name);
			}
			
		}
		(void) closedir(dp);
	}

	allocation_len = num_files + len_files;
	return allocation_len;
}

/* based on the allocation length determined in calc_allocation_len() malloc
a buffer for the message and fills it w/ comma sep dir list. pointer to 
newly malloc'd buffer is returned and it is calling process's job to free it.*/
char* comma_sep_dir_str(int len) {
	char *directory_list = malloc(sizeof(*directory_list) * len);
	DIR *dp;
	struct dirent *ep;

	dp = opendir("./");
	if(dp != NULL) {
		int index = 0; 
		while (ep = readdir(dp)) {
			int i;
			if ((strcmp(ep->d_name, ".") != 0) && (strcmp(ep->d_name, "..") != 0)){
				for(i=0; i < strlen(ep->d_name); i++){
					directory_list[index] = ep->d_name[i];
					index += 1;
				}
				directory_list[index] = ',';
				index += 1;
			}
		}
		(void) closedir(dp);
	}
	directory_list[len-1] = '\0';
	return directory_list;
}
/*in order to work with my system of only 3 chars to describe message len
i need to send a message with the file size or directory buffer size (because
they will surely be longer than just three characters. To do this i make a message
out of the description size. That way i can send an expected size of a number with 
999 digits (dont think that is unreasonalbe). The idea is to take an integer (expected len)
and format my message so for example if expected len is 100, output is 3xx100  
calling processes responsibility to free malloc'd pointer returned.*/
char* format_expected_len_msg(int len){
	char str_of_bytes_expected[999];
	snprintf(str_of_bytes_expected, 998, "%d", len);
	int payload_len = strlen(str_of_bytes_expected);
	char pad_string[4];
	make_pad_string(pad_string, payload_len);
	char *expected_len_formatted = malloc(sizeof(*expected_len_formatted) * (payload_len + 4)); 
	
	int index = 0; 
	int i;
	for(i=0;i<3;i++){
		expected_len_formatted[index] = pad_string[i]; 
		index += 1; 
	}

	for(i=0;i<payload_len;i++){
		expected_len_formatted[index] = str_of_bytes_expected[i];
		index += 1; 
	}
	expected_len_formatted[index] = '\0';
	return expected_len_formatted;
}
//This just makes the pad string (in provided pointer to buffer) of the int provided
// for example: if len = 10 pad string is 10x
void make_pad_string(char *pad_string, int len){
	if (len < 10){
		sprintf(pad_string, "%d", len);
		pad_string[1] = 'x';
		pad_string[2] = 'x';
		pad_string[3] = '\0';
	}
	else if (len < 100){
		sprintf(pad_string, "%d", len);
		pad_string[2] = 'x';
		pad_string[3] = '\0';
	}
	else if (len < 1000){
		sprintf(pad_string, "%d", len);
	}
}
//loop through file and count chars. return len of file name provided
int calc_file_size(char* file_name){
	int file_size_counter = 0;
	FILE *fp;
	fp = fopen(file_name, "r+");
	char c;
	while(( c = fgetc(fp))!= EOF) {
		file_size_counter += 1; 
	}
	fclose(fp);
	return file_size_counter;
}
// break up the file size into (buffer size -1) chunks and send. This 
// should help prevent out of memory errors because the whole file
// does not need to be loaded into memory at one time
void chunks_for_sending(int file_size, char* file_name, int socket_fd){
	int buffer_size = 1025;
	char buffer[buffer_size];
	FILE *fp;
	fp = fopen(file_name, "r+");
	int index = 0;
	int i = 0;
	int bytes_left = file_size; 
	while (i < file_size){
		if (buffer_size - 1 <= bytes_left){
			while (index < (buffer_size - 1)){
				buffer[index] = fgetc(fp);
				index += 1; 
				i+=1;
				bytes_left -= 1; 
			}
			buffer[index] = '\0';
			index = 0;
			send_message(socket_fd, buffer);
		}
		else{
			int j;
			for(j=0;j<=bytes_left;j++){
				buffer[j] = fgetc(fp);
			}
			buffer[bytes_left] ='\0';
			send_message(socket_fd, buffer);
			i=file_size;
		}
	}
	fclose(fp);
}
// simply determine if a file name provided exists for reading. 
int check_if_file_exists(char* file_name){
	FILE *fp;
	fp = fopen(file_name, "r+");
	if (fp == NULL){
		return -1;
	}
	else {
		fclose(fp);
		return 0; 
	}
}
