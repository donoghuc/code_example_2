#! /usr/bin/python3
### Cas Donoghue
### CS372 Project 2
### use: list: python3 ft_client.py [SERVER NAME] [SERVER PORT] [-l] [DATA PORT]
### use: get: python3 ft_client.py [SERVER NAME] [SERVER PORT] [-g] [FILE NAME] [DATA PORT]
### description: client side of FTP interaction. can request a directory list from server or a file to get
import socket
import sys  
import argparse
import os
#my socket class (in socket_class.py)
import socket_class

def main():
	# use argparse module to take care of (most) validation of args
	args = deal_with_argv()
	# list directory requested
	if args.list:
		#try setting up the connections (to server and listening for data)
		try:
			com_connection_client = socket_class.MySocket()
			com_connection_client.client_connect(socket.gethostbyname(args.server_name), args.port_number)
			com_connection_client.client_send("2xxok")
			data_connection_serve = socket_class.MySocket()
			data_connection_serve.server_listen(socket.gethostname(), args.data_port, 1)
		except:
			sys.exit("Connection error, bad host name? bad port?")
		#connections good, send request for list
		print ('Receiving directory structure from {}:{}'.format(args.server_name, args.port_number))
		com_connection_client.client_send(request_FTP_list_dir(socket.gethostname(), str(args.data_port)))
		data_con_confirmation = com_connection_client.client_recv()
		# if server can connect and is all good data_con_confirmation is "ok"
		if data_con_confirmation == 'ok':
			#get expected length of directory over communication channel
			len_directory_data = com_connection_client.client_recv()
			conn, addr = data_connection_serve.server_accept()
			#get the data
			data_connection_serve.recieve_and_print_serv_dir(int(len_directory_data), conn)	
			#clean up
			com_connection_client.client_close()
			data_connection_serve.server_close(conn)
		else: 
			# server did not send "ok" so bad data connection. cleanup and close
			print("FTP server cannot connect to provided data port number...")
			com_connection_client.client_close()
			data_connection_serve.client_close()


	elif args.get:
		#if file allready exists, alert user and see if they want to overwrite, if so or if file does not exist 
		if(check_duplicate_file(args.get)):
			#get connections going
			try:
				com_connection_client = socket_class.MySocket()
				com_connection_client.client_connect(socket.gethostbyname(args.server_name), args.port_number)
				com_connection_client.client_send("2xxok")
				data_connection_serve = socket_class.MySocket()
				data_connection_serve.server_listen(socket.gethostname(), args.data_port, 1)
			except:
				sys.exit("Connection error, bad host name? bad port?")
			#send request of communication socket
			com_connection_client.client_send(request_FTP_file(socket.gethostname(), str(args.data_port), args.get))
			data_con_confirmation = com_connection_client.client_recv()
			#make sure data connection is good from server
			if data_con_confirmation == 'ok':
				#get expected size over communication socket and file data over data socket
				file_bytes_to_recieve = com_connection_client.client_recv()
				conn, addr = data_connection_serve.server_accept()
				#deal with "file not found"
				if file_bytes_to_recieve != 'FILE NOT FOUND':
					print('Receiving "{}" from {}:{}.'.format(args.get, args.server_name, args.port_number))
					data_connection_serve.recieve_and_write_req_file(int(file_bytes_to_recieve), args.get, conn)
					print('File transfer complete')
				elif file_bytes_to_recieve == 'FILE NOT FOUND':
					print("{}:{} says {}".format(args.server_name, args.port_number, file_bytes_to_recieve))
				# clean up
				com_connection_client.client_close()
				data_connection_serve.server_close(conn)
			# data connection on server side failed
			else:
				print("FTP server cannot connect to provided data port number...")
				com_connection_client.client_close()
				data_connection_serve.client_close()
		# client user decided not to overwrite existing file, close w/o ever talking to server. 
		else:
			print("User decided not to overwrite existing file: {}".format(args.get))

# this is the function that returns the argparse object and does the input parameter validation
def deal_with_argv():
	parser = argparse.ArgumentParser()
	parser.add_argument("server_name", help="name of FTP server")
	parser.add_argument("port_number", help="FTP control port number", type=int)
	parser.add_argument("-l", "--list", help="list available files on server", action="store_true")
	parser.add_argument("-g", "--get", help="specify a file to be transferred from server to client", type=str)
	parser.add_argument("data_port", help="port you want data transferred", type=int)
	args = parser.parse_args()
	return args

# this is the prepended system for expected bytes
def pad_message_length(message_len):
	if message_len < 10:
		padded = str(message_len) + 'xx'
	elif message_len < 100:
		padded = str(message_len) + 'x'
	elif message_len < 990:
		padded = str(message_len)
	else:
		return -1
	return padded

# format a list dir request
def request_FTP_list_dir(client_name, data_port):
	data_con_info = '{},{},{}'.format(client_name, data_port, 'list')
	padded_info = pad_message_length(len(data_con_info))
	return padded_info + data_con_info

# format a FTP file request
def request_FTP_file(client_name, data_port, file_name):
	data_con_info = '{},{},get,{}'.format(client_name, data_port, file_name)
	padded_info = pad_message_length(len(data_con_info))
	return padded_info + data_con_info

# check current directory for requested file, if it exists, ask if user wants to overwrite
# return true if file does not allready exist or if user wants to overwrite file
# return false if user does not want to overwrite. 
def check_duplicate_file(file_name):
	path = os.path.abspath(os.path.dirname(__file__))
	dirs = os.listdir(path)
	if file_name in dirs:
		good_response = 0;
		response = input("{} is allready a file in your current directory. \nWant to overwrite [o] or cancel [c] ?: ".format(file_name))
		if (response == 'o') or (response == 'c'):
			good_response = 1; 
		
		while good_response != 1:
			response = input('your entry: {} does not make sense. \nenter [o] to overwrite or [c] to cancel: '.format(response))
			if (response == 'o') or (response == 'c'):
				good_response = 1; 
		if response == 'o':
			return True
		if response == 'c':
			return False
	else:
		return True


if __name__ == '__main__':
    main()