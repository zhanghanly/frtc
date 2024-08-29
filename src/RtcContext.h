#ifndef FRTC_RTC_CONTEXT_H
#define FRTC_RTC_CONTEXT_H

#include <memory>
#include <thread>
#include "DtlsTransport.h"
#include "SrtpSession.h"
#include "RtcTransport.h"
#include "RtcSession.h"
#include "Socket.h"
#include "Ticker.h"

namespace frtc {

class RtcContext : public DtlsTransport::Listener {
public:
    RtcContext();
    
    ~RtcContext();
    
    std::string createLocalSdp();

    void setRemoteSdp(const std::string&);

    void sendRtcpPacket(const char*, int);

    void startConnectPeer();

    void OnDtlsTransportConnecting(const DtlsTransport* dtlsTransport) override;

    void OnDtlsTransportConnected(
         const DtlsTransport* dtlsTransport,
         SrtpSession::CryptoSuite srtpCryptoSuite,
         uint8_t* srtpLocalKey,
         size_t srtpLocalKeyLen,
         uint8_t* srtpRemoteKey,
         size_t srtpRemoteKeyLen,
         std::string& remoteCert) override;

    void OnDtlsTransportFailed(const DtlsTransport* dtlsTransport) override;

    void OnDtlsTransportClosed(const DtlsTransport* dtlsTransport) override;

    void OnDtlsTransportSendData(
         const DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) override;

    void OnDtlsTransportApplicationDataReceived(
         const DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) override;

    void loop(void);

    void stop(void);

    void setFrameCallback(std::function<void(FramePtr)>);

private:
    void send(const char*, int32_t);

    void sendBindRequest(void);

    void onIceData(const char* data, int32_t size);

    void onDtlsData(const char* data, int32_t size);

    void onRtpData(char* data, int32_t size);

    void onRtcpData(char* data, int32_t size);

    void dispatchPeerData(char*, int32_t);

private:
    RtcSessionSp _session;
    SockClientSp _client;
    SrtpSessionSp _srtpSessionSend;
    SrtpSessionSp _srtpSessionRecv;
    DtlsTransportSp _dtlsTransport; 
    RtcTransportSp _transport; 
    TickerSp _ticker;
    std::thread _work;
    bool _runFlag;
};

typedef std::shared_ptr<RtcContext> RtcContextSp;

}

#endif