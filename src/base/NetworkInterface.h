#ifndef FRTC_NETWORK_INTERFACE_H
#define FRTC_NETWORK_INTERFACE_H

#include <string>
#include <memory>
#include <functional> 
#include "httplib.h"

namespace frtc {

enum class SignalProtocol {
    HTTP,
    HTTPS,
    WEBSOCKET,
    SIP,
    NONE
};

enum class SignalErr {
    SUCCESS,
    TIMEOUT,
    STREAM_NOT_FOUND
};

class SignalInterface {
public:
    typedef std::function<void(SignalErr, const std::string&)> readCb;
    
    virtual ~SignalInterface() = default;
    
    void setTimeout(uint32_t);
    
    void setReadCb(readCb);
    
    virtual bool connectPeer(const std::string&) = 0;
    
    virtual void sendReq(const std::string&) = 0;
    
    virtual void disconnect(void) = 0;

protected:
    uint32_t _timeout;
    readCb _readCb;
};

typedef std::shared_ptr<SignalInterface> NetworkSp;

class HttpClient : public SignalInterface {
public:
    typedef std::shared_ptr<httplib::Client> ClientSp;
    
    ~HttpClient() = default;
    
    bool connectPeer(const std::string&) override;
    
    void sendReq(const std::string&) override;
    
    void disconnect(void) override;

private:
    std::string _url;
    uint32_t _port;
    ClientSp _client;
};

NetworkSp createNetworkClient(SignalProtocol);

SignalProtocol parseUrlType(const std::string&);

}

#endif