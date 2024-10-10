#include <iostream>
#include <srtp2/srtp.h>
#include "RtcContext.h"
#include "StunPacket.h"
#include "rtp/RtpPacket.h"
#include "RawBuffer.h"
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
    
RtcContext::RtcContext() : _runFlag(true) {
    _dtlsTransport = std::make_shared<DtlsTransport>(this);
    _transport = std::make_shared<RtcTransport>(this);
    _ticker = std::make_shared<Ticker>();
}

RtcContext::~RtcContext() {
    //if (_client) {
    //    _client->close();
    //}
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
        _client->connect(remoteCandidate->ip, remoteCandidate->port);
        _client->setReadTimeout(200); //0.2s
        LOGI("remoteCandidate.ip=%s remoteCandidate.port=%d", remoteCandidate->ip.c_str(), remoteCandidate->port);
    
        _work = std::thread(&RtcContext::loop, this);
        LOGI("%s", "start loop thread");
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

    if (remoteIce && localIce) {
        char cache[4096];
        buffer->init(cache, 4096);
        stun->encode_request(StunBindingRequest, &(*buffer), *remoteIce, *localIce);
        send(buffer->data(), buffer->pos());
    }
}

void RtcContext::OnDtlsTransportConnecting(const DtlsTransport* dtlsTransport) {
    LOGD("%s", "DTLS handshake is connecting");
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
    LOGE("%s", "DTLS handshake failed");
}

void RtcContext::OnDtlsTransportClosed(const DtlsTransport* dtlsTransport) {
    LOGE("%s", "DTLS handshake failed");
}

void RtcContext::OnDtlsTransportSendData(
    const DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) {
    send((const char*)data, len);
}

void RtcContext::OnDtlsTransportApplicationDataReceived(
    const DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) {
    LOGD("%s", "DTLS handshake application data recieved");
}
    
void RtcContext::onIceData(const char* data, int32_t size) {
    _dtlsTransport->Run(DtlsTransport::Role::CLIENT);
}

void RtcContext::onDtlsData(const char* data, int32_t size) {
    _dtlsTransport->ProcessDtlsData((const uint8_t*)data, size);
}

void RtcContext::onRtpData(char* data, int32_t size) {
    if (_srtpSessionRecv) {
        if (_srtpSessionRecv->DecryptSrtp((uint8_t*)data, &size)) {
            _transport->onRtp(data, size);
        
        } else {
            LOGE("%s", "decrypt rtp packet failed");
        }

    } else {
        LOGE("%s", "not create srtp session yet");
    }
}

void RtcContext::onRtcpData(char* data, int32_t size) {
    if (_srtpSessionRecv) {
        if (_srtpSessionRecv->DecryptSrtcp((uint8_t*)data, &size)) {
            LOGE("%s", "decrypt rtcp packet successfully");
        } else {
            LOGE("%s", "decrypt rtcp packet failed");
        }

    } else {
        LOGE("%s", "not create srtp session yet");
    }
}

void RtcContext::dispatchPeerData(char* data, int32_t size) {
    if (isStun(data, size)) {
        return onIceData(data, size);
    
    } else if (isDtls(data, size)) {
        return onDtlsData(data, size);
    
    } else if (isRtp(data, size)) {
        return onRtpData(data, size);
    
    } else if (isRtcp(data, size)) {
        return onRtcpData(data, size);
    } else {
        LOGI("%s", "unknown type udp data");
    }
}
    
void RtcContext::sendRtcpPacket(const char* data, int len) {
    auto pkt = std::make_shared<BufferRaw>(); 
    // 预留rtx加入的两个字节
    pkt->setCapacity((size_t)len + SRTP_MAX_TRAILER_LEN + 2);
    memcpy(pkt->data(), data, len);
    if (_srtpSessionSend->EncryptRtcp(reinterpret_cast<uint8_t*>(pkt->data()), &len)) {
        pkt->setSize(len);
        send(pkt->data(), pkt->size());

        LOGI("%s", "RtcContext sendRtcpPacket");
    }
}
    
void RtcContext::setFrameCallback(std::function<void(FramePtr)> cb) {
    _transport->setFrameCallback(cb);
}
    
void RtcContext::stop(void) {
    _runFlag = false;
}

void RtcContext::loop() {
    LOGI("%s", "socket thread is looping");
    while (_runFlag) {
        if (_client) {
            LOGI("%s", "loop brfore read");
            _client->read();
            LOGI("%s", "loop after read");
        
            if (_ticker->elapsedTime() > 5000) {
                sendBindRequest();
                _ticker->resetTime();
            }
        }
    }
    LOGI("%s", "socket thread is exiting");
}

}