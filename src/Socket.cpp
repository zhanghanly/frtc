#include <cstring>
#include <iostream>
#include "Socket.h"
#include "Log.h"

namespace frtc {

SockClientSp createSockClient(ClientType type) {
    switch (type) {
    case ClientType::tcp:
        return std::make_shared<TCPClient>();
    case ClientType::udp:
        return std::make_shared<UDPClient>();
    default:
        return nullptr;
    }
}

void SockClient::setRecvCallback(std::function<void(char*, int32_t)> callback) {
    if (callback) {
        _callback = callback;
    }
}

int TCPClient::connect(std::string& peerIp, int32_t port) {
    return 0;
}

int TCPClient::send(const char* data, int32_t size) {
    return 0;
}

void TCPClient::read() {

}

int TCPClient::close() {
    return 0;
}


int UDPClient::connect(std::string& peerIp, int32_t port) {
    _sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (_sockFd == -1) {
        LOG_ERROR("create udp socket failed");
        return -1;
    }
    bzero(&_servAddr, sizeof(_servAddr));
    _servAddr.sin_family = AF_INET;
    _servAddr.sin_port = htons(port);
    _servAddr.sin_addr.s_addr = inet_addr(peerIp.c_str());

    return 0;
}

int UDPClient::send(const char* data, int32_t size) {
    return sendto(_sockFd, data, size, 0, (struct sockaddr*)&_servAddr, sizeof(_servAddr));
}
    
void UDPClient::read() {
    char buffer[4096];
    socklen_t servAddrLen = sizeof(_servAddr);
    int recvLen = recvfrom(_sockFd, buffer, sizeof(buffer), 0, (struct sockaddr*)&_servAddr, &servAddrLen);
    if (recvLen > 0) {
        _callback(buffer, recvLen);
    } else {
        std::cout << "read udp failed" << std::endl;
    }
}

int UDPClient::close() {
    ::close(_sockFd);
    return 0;
}

}