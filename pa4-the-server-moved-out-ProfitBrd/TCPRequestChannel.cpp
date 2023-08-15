#include "TCPRequestChannel.h"

using namespace std;


TCPRequestChannel::TCPRequestChannel (const std::string _ip_address, const std::string _port_no) {
    
    
    //if (server) if we are the server, differenetiate using IP address
    //  create socket on the specified
    //      - specify domain, type, protocol
    //  bind the socket to addr (collect from machine -- getaddrinfo) --> sets up listening
    //  mark socket as listening
    if (_ip_address == ""){
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        bzero((char *)&server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        int temp = stoi(_port_no);
        server_address.sin_port = htons(static_cast<short>(temp));
        bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0;
        listen(sockfd, 5);
    }
    //if client
    //  create a socket on the specified
    //  - specify domain, type, protocol
    //  connect socket ot the IP addr of the server
    else{
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        bzero((char *)&server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        int temp = stoi(_port_no);
        inet_aton(_ip_address.c_str(), &server_address.sin_addr);
        server_address.sin_port = htons(static_cast<short>(temp));
        connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0;
    }
}

TCPRequestChannel::TCPRequestChannel (int _sockfd) {
    this->sockfd = _sockfd;
}

TCPRequestChannel::~TCPRequestChannel () {
    //close the sockfd -- gone over by minh
    close(sockfd);
}

int TCPRequestChannel::accept_conn () {
    //struct sockaddr_storage
    //implementing accept function - return value of the sockfd of client
    newsockfd = accept(sockfd, NULL, NULL);
    return newsockfd;
}

//read/write, recv, send
int TCPRequestChannel::cread (void* msgbuf, int msgsize) {
    return read(this->sockfd, msgbuf, msgsize);
}

int TCPRequestChannel::cwrite (void* msgbuf, int msgsize) {
    return write(this->sockfd, msgbuf, msgsize);
}
