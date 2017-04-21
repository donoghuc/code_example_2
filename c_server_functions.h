#ifndef __C_SERVER_FUNCTIONS_H
#define __C_SERVER_FUNCTIONS_H

int sendall(int s, char *buf, int *len);
void send_message(int sockfd, char *message_to_send);
int recvall(int s, char *buf, int *len);
char* receive_message(int sockfd);
void parse_service_request(char* req_msg, char *data_con_server_name, char *data_con_port_num,
						 char *service_requested, char *file_req);
int calc_allocation_len();
char* comma_sep_dir_str(int len);
char* format_expected_len_msg(int len);
void make_pad_string(char *pad_string, int len);
int calc_file_size(char* file_name);
void chunks_for_sending(int file_size, char* file_name,  int socket_fd);
int check_if_file_exists(char* file_name);


#endif