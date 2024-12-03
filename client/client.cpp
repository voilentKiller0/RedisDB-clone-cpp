#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;

static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

int main()
{
    try
    {

        /*AF_INET: Specifies IPv4 Internet protocols
        SOCK_STREAM: Specifies TCP connection type
        0: Lets the system choose the appropriate protocol (TCP in this case)*/
        int create_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (create_socket < 0)
        {
            throw runtime_error("Socket is not able to create. Status : " + to_string(create_socket));
        }

        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = ntohs(1234);
        addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1
        int rv = connect(create_socket, (const struct sockaddr *)&addr, sizeof(addr));
        if (rv)
        {
            throw runtime_error("Unable to bind the socket. Status : " + to_string(rv));
        }

        while (true)
        {
            string strmsg;
            cout << "Enter Message : ";
            cin >> strmsg;
            if (strmsg == "exit"){
                break;
            }

            // Properly handle the string to char array conversion
            vector<char> msg(strmsg.length() + 1); // +1 for null terminator
            strcpy(msg.data(), strmsg.c_str());

            write(create_socket, msg.data(), strmsg.length());

            char rbuf[64] = {};
            ssize_t n = read(create_socket, rbuf, sizeof(rbuf) - 1);
            if (n < 0)
            {
                throw runtime_error("read() error");
            }
            printf("server says: %s\n", rbuf);
        }
        close(create_socket);
    }
    catch (const exception &e)
    {
        cout << "Exception " << e.what() << endl;
    }
    return 0;
}
