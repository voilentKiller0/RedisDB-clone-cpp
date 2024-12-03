#include <stdint.h>
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

static void do_something(int connfd) {
    while (true) {  // Keep listening for messages from this client
        char rbuf[64] = {};
        ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
        if (n <= 0) {  // Client closed connection or error
            break;
        }
        cout << "client says: " << rbuf << endl;

        char wbuf[] = "got it !!";
        write(connfd, wbuf, strlen(wbuf));
    }
    close(connfd);  // Close the connection when client disconnects
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

			do_something(connfd);			
		}
		close(create_socket);

	}catch(const exception& e){
		cout << "Exception " << e.what() << endl;
	}
	return 0;
}