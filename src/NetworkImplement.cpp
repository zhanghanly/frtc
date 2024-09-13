#include <cstring>
#include <iostream>
#include "NetworkInterface.h"
#include "nlohmann/json.hpp"
#include "Utility.h"
#include "Log.h"

namespace frtc {

using json = nlohmann::json;
    
NetworkSp createNetworkClient(SignalProtocol protocol) {
    switch (protocol) {
    case SignalProtocol::HTTP:
    case SignalProtocol::HTTPS:
        return  std::make_shared<HttpClient>();
    case SignalProtocol::SIP:
        return  nullptr;
    case SignalProtocol::WEBSOCKET:
        return  nullptr;
    default:
        return nullptr;
    }
}

SignalProtocol parseUrlType(const std::string& url) {
    if (url.find("https") == 0) {
        return SignalProtocol::HTTPS;
    } else if (url.find("http") == 0) {
        return SignalProtocol::HTTP;
    } else if (url.find("sip") == 0) {
        return SignalProtocol::SIP;
    } else if (url.find("websocket") == 0) {
        return SignalProtocol::WEBSOCKET;
    } else {
        return SignalProtocol::NONE;
    }
}
    
void SignalInterface::setTimeout(uint32_t timeout) {
    _timeout = timeout;
}

void SignalInterface::setReadCb(readCb cb) {
    _readCb = cb;
}

bool HttpClient::connectPeer(const std::string& address) {
    if (!_client) {
        //eg: https://video.whatsgps.com:1443/47.236.232.146-20443/index/api/webrtc?app=jtt1078&stream=020091590684_1_0&type=play"
        std::vector<std::string> vec = splitStrWithSeparator(address.c_str() + strlen("http://"), "/"); 
        if (vec.empty()) {
            LOGE("bad https=%s address", address.c_str());
            return false;
        }     
        
        std::vector<std::string> urls = splitStrWithSeparator(address, vec[0]); 
        if (urls.size() > 1) {
            _url = urls[1];
        }
        
        std::vector<std::string> dominAndPort = splitStrWithSeparator(vec[0], ":"); 
        if (dominAndPort.size() != 2) {
            dominAndPort.push_back("80");
        }

        _client = std::make_shared<httplib::Client>(dominAndPort[0], std::atoi(dominAndPort[1].c_str()));
    }

    return true;
}

void HttpClient::sendReq(const std::string& body) {
    httplib::Result res = _client->Post(_url.c_str(), body.c_str(), body.size(), "text/plain;charset=UTF-8");
    if (res) {
        json j = json::parse(res->body);
        for (auto& iter : j.items()) {
            if (iter.key() == "sdp") {
                if (_readCb) {
                    _readCb(SignalErr::SUCCESS, iter.value());
                }
                break;
            }
        } 
    
    } else {
        //if (_readCb) {
        //    _readCb(SignalErr::TIMEOUT, res.error());
        //}
        LOGE("%s", "http request failed");
    }
}

void HttpClient::disconnect(void) {

}

}
