#include <iostream>
#include "RtcContext.h"
#include "StunPacket.h"
#include "rtp/RtpPacket.h"
#include "Log.h"

namespace frtc {

bool isStun(const char* data, int32_t size) {
    return // STUN headers are 20 bytes.
           (size >= 20) &&
           // DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
           (data[0] < 3) &&
           // Magic cookie must match.
           (data[4] == (char)0x21) && 
           (data[5] == (char)0x12) &&
           (data[6] == (char)0xa4) && 
           (data[7] == (char)0x42);
}

bool isDtls(const char* data, int32_t size) {
    return ((*data > 19) && (*data < 64));
}

bool isRtp(const char* data, int32_t size) {
    RtpHeader* header = (RtpHeader*)data;
    return ((header->pt < 64) || (header->pt >= 96));
}

bool isRtcp(const char* data, int32_t size) {
    RtpHeader* header = (RtpHeader*)data;
    return ((header->pt >= 64) && (header->pt < 96));
}
    
RtcContext::RtcContext() {
    _dtlsTransport = std::make_shared<DtlsTransport>(this);
    _transport = std::make_shared<RtcTransport>();
    _ticker = std::make_shared<Ticker>();
    std::cout << "start thread\n";
    _work = std::thread(&RtcContext::loop, this);
}

RtcContext::~RtcContext() {
    if (_work.joinable()) {
        _work.join();
    }
 }

std::string RtcContext::createLocalSdp() {
    if (!_session) {
        std::string localFingerprint;
        for (auto& fingerprint : _dtlsTransport->GetLocalFingerprints()) {
            if (fingerprint.algorithm == DtlsTransport::FingerprintAlgorithm::SHA256) {
                localFingerprint = fingerprint.value;
            }
        }
        _session = std::make_shared<RtcSession>(localFingerprint);
    }

    return _session->createLocalSdp();
}

void RtcContext::setRemoteSdp(const std::string& remoteSdp){
    if (_session) {
        _session->setRemoteSdp(remoteSdp);
        // 设置远端dtls签名
        frtc::DtlsTransport::Fingerprint remoteFingerprint;
        remoteFingerprint.algorithm = DtlsTransport::FingerprintAlgorithm::SHA256;
        remoteFingerprint.value = _session->getRemoteFingerprint(); 
        _dtlsTransport->SetRemoteFingerprint(remoteFingerprint); 
        // transport load sdp
        _transport->loadSdp(_session->getRtmoteSdp());
    }
}
    
void RtcContext::startConnectPeer() {
    std::cout << "now get remote candidate message" << std::endl;
    CandidatePtr remoteCandidate = _session->getCandidate(); 
    if (remoteCandidate && !_client) {
        //Candidate candidate = _session->getCandidate();
        ClientType clientType = ClientType::udp;
        //if (candidate.type == "tcp") {
        //    clientType = ClientType::tcp;
        //} 
        _client = createSockClient(clientType);
        _client->setRecvCallback([this](char* data, uint32_t size) {
            dispatchPeerData(data, size);
        });
        
        std::cout << "remoteCandidate.ip remoteCandidate.port" << remoteCandidate->ip << " " << remoteCandidate->port << std::endl;
        _client->connect(remoteCandidate->ip, remoteCandidate->port);
    }
    sendBindRequest();
}

void RtcContext::send(const char* data, int32_t size) {
    _client->send(data, size);
}
    
void RtcContext::sendBindRequest(void) {
    StunLibSp stun = std::make_shared<StunLib>();
    RWBufferSp buffer = std::make_shared<RWBuffer>();
    SessionInfoPtr remoteIce = _session->getRemoteIceInfo();
    SessionInfoPtr localIce = _session->getLocalIceInfo();

    char cache[4096];
    buffer->init(cache, 4096);
    stun->encode_request(StunBindingRequest, &(*buffer), *remoteIce, *localIce);
    send(buffer->data(), buffer->pos());
}

void RtcContext::OnDtlsTransportConnecting(const DtlsTransport* dtlsTransport) {
    LOG_DEBUG("%s", "DTLS handshake is connecting");
    std::cout << "DTLS handshake is connecting" << std::endl;
}

void RtcContext::OnDtlsTransportConnected(
     const DtlsTransport* dtlsTransport,
     SrtpSession::CryptoSuite srtpCryptoSuite,
     uint8_t* srtpLocalKey,
     size_t srtpLocalKeyLen,
     uint8_t* srtpRemoteKey,
     size_t srtpRemoteKeyLen,
     std::string& remoteCert) {
    _srtpSessionSend = std::make_shared<SrtpSession>(
        SrtpSession::Type::OUTBOUND, srtpCryptoSuite, srtpLocalKey, srtpLocalKeyLen);
    _srtpSessionRecv = std::make_shared<SrtpSession>(
        SrtpSession::Type::INBOUND, srtpCryptoSuite, srtpRemoteKey, srtpRemoteKeyLen);
}

void RtcContext::OnDtlsTransportFailed(const DtlsTransport* dtlsTransport) {
    LOG_ERROR("%s", "DTLS handshake failed");
    std::cout << "DTLS handshake failed" << std::endl;
}

void RtcContext::OnDtlsTransportClosed(const DtlsTransport* dtlsTransport) {
    LOG_ERROR("%s", "DTLS handshake failed");
    std::cout << "DTLS handshake failed" << std::endl;
}

void RtcContext::OnDtlsTransportSendData(
    const DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) {
    send((const char*)data, len);
}

void RtcContext::OnDtlsTransportApplicationDataReceived(
    const DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) {
    LOG_DEBUG("%s", "DTLS handshake application data recieved");
    std::cout << "DTLS handshake application data recieved" << std::endl;
}
    
void RtcContext::onIceData(const char* data, int32_t size) {
    std::cout << "recieve ice data" << std::endl;
    _dtlsTransport->Run(DtlsTransport::Role::CLIENT);
}

void RtcContext::onDtlsData(const char* data, int32_t size) {
    std::cout << "recieve dtls data" << std::endl;
    _dtlsTransport->ProcessDtlsData((const uint8_t*)data, size);
}

void RtcContext::onRtpData(char* data, int32_t size) {
    std::cout << "recieve rtp data" << std::endl;
    if (_srtpSessionRecv) {
        if (_srtpSessionRecv->DecryptSrtp((uint8_t*)data, &size)) {
            std::cout << "decrypt rtp packet successfully" << std::endl;
            _transport->onRtp(data, size);
        
        } else {
            std::cout << "decrypt rtp packet failed" << std::endl;
        }

    } else {
        std::cout << "not create srtp session yet" << std::endl;
    }
}

void RtcContext::onRtcpData(char* data, int32_t size) {
    std::cout << "recieve rtcp data" << std::endl;
    if (_srtpSessionRecv) {
        if (_srtpSessionRecv->DecryptSrtcp((uint8_t*)data, &size)) {
            std::cout << "decrypt rtcp packet successfully" << std::endl;
        } else {
            std::cout << "decrypt rtcp packet failed" << std::endl;
        }

    } else {
        std::cout << "not create srtp session yet" << std::endl;
    }
}

void RtcContext::dispatchPeerData(char* data, int32_t size) {
    std::cout << "dispatch recieve data" << std::endl;
    if (isStun(data, size)) {
        return onIceData(data, size);
    
    } else if (isDtls(data, size)) {
        return onDtlsData(data, size);
    
    } else if (isRtp(data, size)) {
        return onRtpData(data, size);
    
    } else if (isRtcp(data, size)) {
        return onRtcpData(data, size);
    }
}

void RtcContext::loop() {
    std::cout << "looping" << std::endl;
    while (true) {
        if (_client) {
            _client->read();
        
            if (_ticker->elapsedTime() > 5000) {
                sendBindRequest();
                _ticker->resetTime();
            }
        }
    }
}

}