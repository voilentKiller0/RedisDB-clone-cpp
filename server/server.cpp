#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdexcept>

using namespace std;

const size_t k_max_msg = 4096;

int32_t read_full(int conn_fd, char *buff, size_t n){
	while (n > 0){
		ssize_t rv = read(conn_fd, buff, n);
		if (rv <= 0){
			return -1; // return -1 when i got enexpcted EOF or error
		}
		assert((size_t)rv <= n);
		n -= size_t(rv);
		buff += rv;
	}
	return 0;
}

int32_t write_full(int conn_fd, const char *buff, size_t n){
	while (n > 0){
		ssize_t wv = write(conn_fd, buff, n);
		if (wv <= 0){
			return -1; // error
		}
		assert((size_t)wv <= n);
		n -= (size_t)wv;
		buff += wv;
	}
	return 0;
}

int32_t one_request(int conn_fd){
	// 4 bytes header
	char rbuff[4+k_max_msg+1];
	errno = 0;
	if (int32_t err = read_full(conn_fd, rbuff, 4)){
		if (err == 0){
			cout << "EOF detected" << endl;
		}else{
			throw runtime_error("Error in read()");
		}
		return err;
	}		
	uint32_t len = 0;
	memcpy(&len, rbuff, 4);
	if (len > k_max_msg){
		throw runtime_error("Message length is too long...");
	}


	// request body
	if (int32_t err = read_full(conn_fd, &rbuff[4], len)){
		if (err == 0){
			cout << "EOF detected" << endl;
		}else{
			throw runtime_error("Error in read()");
		}
		return err;
	}


	// do something
	rbuff[4+len] = '\0';
	cout << "client says: " << &rbuff[4] << endl;

	const char reply[] = "THIS MESSAGE FROM SERVER OK !!";

	char wbuff[4+sizeof(reply)];
	len = (uint32_t)strlen(reply);
	memcpy(wbuff, &len, 4);
	memcpy(&wbuff[4], reply, len);
	return write_full(conn_fd, wbuff, 4+len);
}


int main(){

	try{
		/*AF_INET: Specifies IPv4 Internet protocols
		SOCK_STREAM: Specifies TCP connection type
		0: Lets the system choose the appropriate protocol (TCP in this case)*/
		int create_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (create_socket < 0){
			throw runtime_error("Socket is not able to create. Status : " + to_string(create_socket));
		}

		/*Setting socket options with setsockopt() - this allows the server to reuse the same address if it needs to restart*/
		int val = 1;
    	setsockopt(create_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

		// bind
		// Associates the socket with a specific address and port
		struct sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = ntohs(1234);		// specify the which port is use
		addr.sin_addr.s_addr = ntohl(0);    // wildcard address 0.0.0.0
		int rv = bind(create_socket, (const sockaddr *)&addr, sizeof(addr));
		if (rv){
			throw runtime_error("Unable to bind the socket. Status : " + to_string(rv));
		}


		// listen
		//  Marks the socket as passive, ready to accept incoming connections
		// SOMAXCONN: Maximum length of the queue of pending connections (4096)
		rv = listen(create_socket, SOMAXCONN);
		if (rv) {
			throw runtime_error("Unable to Listen the address. Status : " + to_string(rv));
		}

		while (true) {
			// accept
			struct sockaddr_in client_addr = {};
			socklen_t socklen = sizeof(client_addr);
			/*fd: The listening socket file descriptor
			(struct sockaddr *)&client_addr: Pointer to store client's address information
			&socklen: Pointer to the size of the address structure*/
			int connfd = accept(create_socket, (struct sockaddr *)&client_addr, &socklen);
			if (connfd < 0) {
				continue;   // error
			}

			while (true){
				int32_t err = one_request(connfd);
				if (err){
					break;
				}
			}		
		}
		close(create_socket);

	}catch(const exception& e){
		cout << "Exception " << e.what() << endl;
	}
	return 0;
}