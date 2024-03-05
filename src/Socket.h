#ifndef FRTC_SOCKET_H
#define FRTC_SOCKET_H

#include <memory>
#include <string>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

namespace frtc {

enum class ClientType {
    udp,
    tcp
};

class SockClient {
public:
    virtual ~SockClient() = default;

    virtual int connect(std::string& peerIp, int32_t port) = 0;

    virtual int send(const char* data, int32_t size) = 0;

    virtual void read() = 0;

    virtual int close() = 0;

    void setRecvCallback(std::function<void(char*, int32_t)>);

protected:
    std::function<void(char*, int32_t)> _callback;
};

typedef std::shared_ptr<SockClient> SockClientSp;


class TCPClient : public SockClient {
public:
    ~TCPClient() override = default;

    int connect(std::string& peerIp, int32_t port) override;

    int send(const char* data, int32_t size) override;
    
    void read() override;

    int close() override;
};


class UDPClient : public SockClient {
public:
    ~UDPClient() = default;

    int connect(std::string& peerIp, int32_t port) override;

    int send(const char* data, int32_t size) override;
    
    void read() override;

    int close() override;

private:
    int _sockFd;
    struct sockaddr_in _servAddr;
};

SockClientSp createSockClient(ClientType);

}

#endif