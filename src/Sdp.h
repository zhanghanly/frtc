#ifndef FRTC_SDP_H
#define FRTC_SDP_H

#include <string>
#include <memory>
#include <vector>
#include <map>

namespace frtc {

static const char* gExtMap[] = {
	"urn:ietf:params:rtp-hdrext:toffset",
	"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time",
	"urn:3gpp:video-orientation",
	"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01",
	"http://www.webrtc.org/experiments/rtp-hdrext/playout-delay",
	"http://www.webrtc.org/experiments/rtp-hdrext/video-content-type",
	"http://www.webrtc.org/experiments/rtp-hdrext/video-timing",
	"http://www.webrtc.org/experiments/rtp-hdrext/color-space",
	"urn:ietf:params:rtp-hdrext:sdes:mid",
	"urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id",
	"urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id",
	"urn:ietf:params:rtp-hdrext:ssrc-audio-level"
};

struct SessionInfo {
	std::string iceUfrag;
	std::string icePwd;
	std::string iceOptions;
	std::string fingerprintAlgo;
	std::string fingerprint;
	std::string setup;
};

typedef std::shared_ptr<SessionInfo> SessionInfoPtr;

struct MediaPayload {
	int32_t payloadType;
	int32_t clockRate;
	std::string encodingName;
	std::string encodingParam;
	std::string formatSpecificParam;
	std::vector<std::string> rtcpFbs;
};

typedef std::shared_ptr<MediaPayload> MediaPayloadPtr;

struct SSRCInfo {
	uint32_t ssrc;
	uint32_t rtx_ssrc;
    std::string cname;
    std::string msid;
    std::string mslabel;
    std::string label;
};

typedef std::shared_ptr<SSRCInfo> SSRCInfoPtr;

struct Candidate {
	int32_t port;
	std::string ip;
	std::string type;
	std::string protocol;
};

typedef std::shared_ptr<Candidate> CandidatePtr;

struct MediaDesc {
	MediaDesc();
	
	int32_t port;
	int32_t sctp_port;
	int32_t rtcp_mux;
	int32_t rtcp_rsize;

	int32_t inactive;
	std::string direct;
	std::string mux;
	std::string rsize;
	std::string type;
	std::string mid;
	std::string msid;
	std::string msidTracker;
	std::string protos;
	std::string iceMode;
	bool support_rtx = false;

	SSRCInfoPtr ssrc;	
	CandidatePtr candidate;
	SessionInfoPtr sessionInfo;
	std::map<int32_t, std::string> extmap;
	std::vector<MediaPayloadPtr> payloads;
	std::vector<uint32_t> ssrc_group;
};

typedef std::shared_ptr<MediaDesc> MediaDescPtr;

class Sdp {
public:
	std::string dump();
	void parse(const std::string&);
	std::string create(const std::string&);

	int64_t startTime;
	int64_t endTime;
	std::string version;
	std::string userName;
	std::string sessionId;
	std::string sessionVersion;
	std::string netType;
	std::string addrType;
	std::string unicastAddress;
	std::string sessionName;
	SessionInfo sessionInfo;
	std::string groupPolicy;
	//MediaDesc videoDesc;
	//MediaDesc audioDesc;
	std::vector<MediaDescPtr> medias;

	MediaDescPtr videoDesc(void);
	MediaDescPtr audioDesc(void);

private:
	std::string mediaTypeLst(const MediaDescPtr);	
	std::string payloadInfoLst(const MediaDescPtr); 
};

typedef std::shared_ptr<Sdp> SdpSp;

}

#endif
