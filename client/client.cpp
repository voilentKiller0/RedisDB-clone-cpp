#include <stdint.h>
#include <assert.h>
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
const size_t k_max_msg = 4096;

static int32_t read_full(int fd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) {
            return -1;  // error, or unexpected EOF
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1;  // error
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}


static int32_t query(int fd, const char *text){
    // send the message
    uint32_t len = (uint32_t) strlen(text);
    if (len > k_max_msg){
        throw runtime_error("Message length is too long");
    }
    char wbuff[4 + k_max_msg];
    memcpy(wbuff, &len, 4);
    memcpy(&wbuff[4], text, len);
    if (int32_t err = write_all(fd, wbuff, 4 + len)) {
        return err;
    }


    // recieve the message
    char rbuff[4 + k_max_msg + 1];
    errno = 0;
    int32_t err = read_full(fd, rbuff, 4);
    if (err){
        if (err == 0){
            cout << "Unexpected EOF read" << endl;
        }else{
            throw runtime_error("Error in read()");
        }
        return err;
    }
    memcpy(&len, rbuff, 4);
    if (len > k_max_msg){
        throw runtime_error("Message is too long");
    }
    err = read_full(fd, &rbuff[4], len);
    if (err){
        if (err == 0){
            cout << "Unexpected EOF read" << endl;
        }else{
            throw runtime_error("Error in read()");
        }
        return err;
    }
    // do something
    rbuff[4 + len] = '\0';
    cout << "Server says : " << &rbuff[4] << endl;
    return 0;
}

int main()
{
    try
    {

        /*AF_INET: Specifies IPv4 Internet protocols
        SOCK_STREAM: Specifies TCP connection type
        0: Lets the system choose the appropriate protocol (TCP in this case)*/
        int conn_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (conn_fd < 0)
        {
            throw runtime_error("Socket is not able to create. Status : " + to_string(conn_fd));
        }

        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = ntohs(1234);
        addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1
        int rv = connect(conn_fd, (const struct sockaddr *)&addr, sizeof(addr));
        if (rv)
        {
            throw runtime_error("Unable to bind the socket. Status : " + to_string(rv));
        }

        // send Multiple request

        while (true){
            string msg;
            getline(cin, msg);
            if (msg == "n"){
                break;
            }
            int n = msg.length();
            char arr_msg[n+1];
            strcpy(arr_msg, msg.c_str()); 
            arr_msg[n] = '\0';
            int32_t err = query(conn_fd, arr_msg);
            if (err < 0){
                break;
            }
        }
        close(conn_fd);
    }
    catch (const exception &e)
    {
        cout << "Exception " << e.what() << endl;
    }
    return 0;
}
