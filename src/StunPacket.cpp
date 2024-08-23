#include <cstring>
#include <iostream>
#include "StunPacket.h"
#include "EncryptionAlgo.h"
#include "Utility.h"
#include "Log.h"

namespace frtc {

RWBufferSp stun_encode_hmac2(char* hmac_buf, const int32_t hmac_buf_len) {
	char buf[1460];
	uint32_t ret=0;
	
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes(StunMessageIntegrity);
	stream->write2bytes(hmac_buf_len);
	stream->writeBytes(hmac_buf, hmac_buf_len);
	
	return stream;
}

RWBufferSp stun_encode_fingerprint2(uint32_t crc32) {
	char buf[1460];
	uint32_t ret=0;
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes((int16_t)StunFingerprint);
	stream->write2bytes(4);
	stream->write4bytes(crc32);
	
	return stream;
}

RWBufferSp stun_encode_rtcusername(StunPacket* pkt) {
	char buf[1460];
	uint32_t ret=0;
	char username[128];
	sprintf(username, "%s:%s", pkt->remote_ufrag.c_str(), pkt->local_ufrag.c_str());

	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes(StunUsername);
	stream->write2bytes(strlen(username));
	stream->writeBytes(username, strlen(username));
	if (stream->pos() % 4 != 0) {
		char padding[4] = {0};
		stream->writeBytes(padding, 4 - (stream->pos() % 4));
	}
	
	return stream;
}

RWBufferSp stun_encode_mapped_address_ipv6(StunPacket* pkt,struct sockaddr_in6* addr) {
	char buf[1460];
	uint32_t ret=0;
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes(StunXorMappedAddress);
	stream->write2bytes(20);
	stream->write1bytes(0); // ignore this bytes
	stream->write1bytes(2); // ipv6 family
	stream->write2bytes(pkt->mapped_port ^ (kStunMagicCookie >> 16));
	unsigned char cookie[4]={0x21,0x12,0xA4,0x42};
	//uint8_t* p=(uint8_t*)(&addr->sin6_addr.s6_addr);
	//int32_t i=0;
	//for(i=0; i<4; i++){
	//	stream->write1bytes(p[i]^ cookie[i]);
	//}
	//for(i=0;i<12;i++){
	//	stream->write1bytes(p[i+4]^ pkt->transcation_id[i]);
	//}

	return stream;
}

//int32_t stun_encode_binding_request(StunPacket* pkt, RWBuffer* stream,char* pwd,char* username,uint32_t username_len) {
//	stream->write2bytes(StunBindingRequest);
//	stream->write2bytes(username_len);// +mapped_address_len);
//	stream->write4bytes(kStunMagicCookie);
//	stream->writeBytes(pkt->transcation_id, strlen(pkt->transcation_id));
//	stream->writeBytes(username,username_len);
//	
//	char* randstr=(char*)yang_calloc(1,32);//[16];
//	//Priority
//	yang_cint32_random(30,randstr+1);
//	randstr[0]=9;
//	uint32_t randint=atoi(randstr);
//	stream->write2bytes(StunPriority);
//	stream->write2bytes(4);
//	stream->write4bytes(randint);
//	//UseCandidate
//	stream->write2bytes(StunUseCandidate);
//	stream->write2bytes(0);
//
//	//IceControlling
//	yang_memset(randstr,0,32);
//	yang_cstr_random(8, randstr);
//	stream->write2bytes((int16_t)StunIceControlling);
//	stream->write2bytes(8);
//	stream->writeBytes(randstr,8);
//	yang_free(randstr);
//
//	stream->data()[2] = ((stream->pos() - 20 + 20 + 4 ) & 0x0000FF00) >> 8;
//	stream->data()[3] = ((stream->pos() - 20 + 20 + 4 ) & 0x000000FF);
//#if Yang_Enable_Dtls
//	char hmac_buf[20] = {0};
//	uint32_t  hmac_buf_len = 0;
//	if ((err = hmac_encode("sha1", pwd, yang_strlen(pwd), stream->data, yang_buffer_pos(stream), hmac_buf, &hmac_buf_len)) != Yang_Ok) {
//		return yang_error_wrap(err, "hmac encode failed");
//	}
//
//#else
//	char hmac_buf[20]={0x07,0xd4,0x3d,0x32,0xa1,0xd4,0xc1,0xb1,0x9d,0xf5,0xb5,0x56,0xb5,0x56,0x6d,0x20,0x5a,0xda,0xa1,0xac};
//	uint32_t  hmac_buf_len = 20;
//#endif
//
//	//MessageIntegrity
//	RWBufferSp hmac = stun_encode_hmac2(hmac_buf, hmac_buf_len);
//	stream->writeBytes(hmac->data(), hmac->size());
//	stream->data()[2] = ((stream->pos() - 20 + 8) & 0x0000FF00) >> 8;
//	stream->data()[3] = ((stream->pos() - 20 + 8) & 0x000000FF);
//
//	//Fingerprint
//	uint32_t crc32 = yang_crc32_ieee(stream->data(), stream->pos(), 0)^ 0x5354554E;
//	RWBufferSp fingerprint = stun_encode_fingerprint2(crc32);
//	stream->writeBytes(fingerprint->data(), fingerprint->size());
//	stream->data()[2] = ((stream->pos() - 20) & 0x0000FF00) >> 8;
//	stream->data()[3] = ((stream->pos() - 20) & 0x000000FF);
//}

//int32_t stun_encode_binding_response(StunPacket* pkt, char* pwd, RWBuffer* stream) {
//	RWBufferSp property_username = stun_encode_rtcusername(pkt);
//	RWBufferSp mapped_address = stun_encode_mapped_address_ipv4(pkt);
//
//	stream->write2bytes(StunBindingSuccessResponse);
//	stream->write2bytes(property_username_len + mapped_address_len);
//	stream->write4bytes(kStunMagicCookie);
//	stream->writeBytes(pkt->transcation_id,12);
//	stream->writeBytes(property_username,property_username_len);
//	stream->writeBytes(mapped_address,mapped_address_len);
//	stream->data()[2] = ((stream->pos() - 20 + 20 + 4) & 0x0000FF00) >> 8;
//	stream->data()[3] = ((stream->pos() - 20 + 20 + 4) & 0x000000FF);
//
//#if Yang_Enable_Dtls
//	char hmac_buf[20] = {0};
//	uint32_t  hmac_buf_len = 0;
//	if ((err = hmac_encode("sha1", pwd, yang_strlen(pwd), stream->data, yang_buffer_pos(stream), hmac_buf, &hmac_buf_len)) != Yang_Ok) {
//		return yang_error_wrap(err, "hmac encode failed");
//	}
//#else
//	char hmac_buf[20]={0x07,0xd4,0x3d,0x32,0xa1,0xd4,0xc1,0xb1,0x9d,0xf5,0xb5,0x56,0xb5,0x56,0x6d,0x20,0x5a,0xda,0xa1,0xac};
//	uint32_t  hmac_buf_len = 20;
//#endif
//
//	RWBufferSp hmac = stun_encode_hmac2(hmac_buf, hmac_buf_len);
//	stream->writeBytes(hmac->data(), hmac->size());
//	stream->data()[2] = ((stream->pos() - 20 + 8) & 0x0000FF00) >> 8;
//	stream->data()[3] = ((stream->pos() - 20 + 8) & 0x000000FF);
//
//	uint32_t crc32 = crc32_ieee(stream->data(), stream->pos(), 0) ^ 0x5354554E;
//	RWBufferSp fingerprint = stun_encode_fingerprint2(crc32);
//	stream->writeBytes(fingerprint->data(),fingerprint->size());
//	stream->data()[2] = ((stream->pos() - 20) & 0x0000FF00) >> 8;
//	stream->data()[3] = ((stream->pos() - 20) & 0x000000FF);
//}

int32_t stun_encode_binding_request2(StunPacket* pkt, RWBuffer* stream, const char* pwd, const char* username, uint32_t username_len){
	//write_2bytes(stream,BindingRequest);
	stream->write2bytes(pkt->message_type);
	stream->write2bytes(username_len); //+mapped_address_len);
	stream->write4bytes(kStunMagicCookie);
	stream->writeBytes((pkt->transcation_id).c_str(), (pkt->transcation_id).size());
	stream->writeBytes(username, username_len);

	std::string randDigitStr = generateRandDigit(10);
	uint32_t randint=std::atoi(randDigitStr.c_str());
	stream->write2bytes(StunPriority);
	stream->write2bytes(4);
	stream->write4bytes(randint);

	//Candidate
	stream->write2bytes((int16_t)StunUseCandidate);
	stream->write2bytes(0);

	//IceControlling
	std::string randMixedStr = generateRandMixedStr(8);
	stream->write2bytes((int16_t)StunIceControlling);
	stream->write2bytes(8);
	stream->writeBytes(randMixedStr.c_str(), randMixedStr.size());
	stream->data()[2] = ((stream->pos() - 20 + 20 + 4 ) & 0x0000FF00) >> 8;
	stream->data()[3] = ((stream->pos() - 20 + 20 + 4 ) & 0x000000FF);

	char hmac_buf[20] = {0};
	uint32_t  hmac_buf_len = 0;
	if (hmac_encode("sha1", pwd, strlen(pwd), stream->data(), stream->pos(), hmac_buf, &hmac_buf_len) != 0) {
		return -1;
	}
	
	//MessageIntegrity
	RWBufferSp hmac = stun_encode_hmac2(hmac_buf, hmac_buf_len);
	stream->writeBytes(hmac->data(), hmac->pos());
	stream->data()[2] = ((stream->pos() - 20 + 8) & 0x0000FF00) >> 8;
	stream->data()[3] = ((stream->pos() - 20 + 8) & 0x000000FF);

	uint32_t crc32 = crc32_ieee(stream->data(), stream->pos(), 0)^ 0x5354554E;
	RWBufferSp fingerprint = stun_encode_fingerprint2(crc32);
	stream->writeBytes(fingerprint->data(), fingerprint->pos());
	stream->data()[2] = ((stream->pos() - 20) & 0x0000FF00) >> 8;
	stream->data()[3] = ((stream->pos() - 20) & 0x000000FF);

	return 0;
}

//int32_t RtcStun::encodeStunServer(RWBuffer* stream,void* pudp,char* username,char* ice_pwd) {
//	YangRtcSocket* udp=(YangRtcSocket*)pudp;
//
//	char tid[13];
//	yang_memset(tid,0,13);
//	yang_cstr_random(12,tid);
//    char localIp[128]={0};
//    yang_getLocalInfo(udp->session.familyType,localIp);
//    uint32_t addr = yang_be32toh(yang_inet_addr(localIp));
//
//	YangStunPacket packet;
//	yang_memset(&packet,0,sizeof(YangStunPacket));
//	packet.message_type=stunType;;
//	yang_strcpy(packet.transcation_id,tid);
//	packet.mapped_address=addr;
//    packet.mapped_port=yang_addr_getSinPort(&udp->session.local_addr);
//
//	char* property_username=NULL;
//	uint32_t property_username_len=yang_stun_encode_rtcusername(&packet,&property_username);
//	yang_stun_encode_binding_request2(&packet,stream,udp,ice_pwd,property_username,property_username_len);
//	yang_free(property_username);
//
//	return Yang_Ok;
//}

int32_t RtcStun::decodeStunServer(StunPacket* pkt,char* buf, const int32_t nb_buf) {
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, nb_buf);
	if (stream->left() < 20) {
		LOG_ERROR("ERROR_RTC_STUN invalid stun packet, size=%d", stream->size());
		return -1;
	}

	pkt->message_type = stream->read2bytes();
	uint16_t message_len = stream->read2bytes();
	char magic_cookie[4] ;
	char transcation_id[12] ;
	stream->readBytes(magic_cookie,4);
	stream->readBytes(transcation_id,12);
	pkt->transcation_id = std::string(transcation_id, 12);
	if (nb_buf != 20 + message_len) {
		LOG_ERROR("ERROR_RTC_STUN invalid stun packet, message_len=%d, nb_buf=%d", message_len, nb_buf);
		return -1;
	}

	while (stream->left() >= 4) {
		uint16_t type = stream->read2bytes();
		uint16_t len = stream->read2bytes();
		if (stream->left() < len) {
			LOG_ERROR("invalid stun packet");
			return -1;
		}

		char* val = new char[len];
		stream->readBytes(val,len);
		// padding
		if (len % 4 != 0) {
			stream->skip(4 - (len % 4));
		}
		switch (type) {
		case StunUsername: {
			pkt->username = std::string(val,len);
			char* p = strtok(val, ":");
			if(p) {
				//strcpy(pkt->local_ufrag, p);
				pkt->local_ufrag = p;
				p = strtok(NULL, ":");
				if(p) {
					//strcpy(pkt->remote_ufrag ,p);
					pkt->remote_ufrag = p;
				}
			}
			break;
		}

		case StunUseCandidate: {
			pkt->use_candidate = true;
			//yang_trace("stun use-candidate");
			break;
		}
		case StunXorMappedAddress:{
			//family=*(val+1)==1?IPv4:IPv6;
			//pkt->mapped_port = get_be16((uint8_t*)val+2)^ ((uint16_t)(kStunMagicCookie >> 16));
			//pkt->mapped_address = get_be32((uint8_t*)val+4)^ kStunMagicCookie;
			break;
		}
		// @see: https://tools.ietf.org/html/draft-ietf-ice-rfc5245bis-00#section-5.1.2
		// One agent full, one lite:  The full agent MUST take the controlling
		// role, and the lite agent MUST take the controlled role.  The full
		// agent will form check lists, run the ICE state machines, and
		// generate connectivity checks.
		case StunIceControlled: {
			pkt->ice_controlled = true;
			//yang_trace("stun ice-controlled");
			break;
		}
		case StunIceControlling: {
			pkt->ice_controlling = true;
			//yang_trace("stun ice-controlling");
			break;
		}
		default: {
			break;
		}
		}
	}

	return 0;
}

int32_t RtcStun::decode(StunPacket* pkt, char* buf,  int32_t nb_buf) {
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, nb_buf);
	if (stream->left() < 20) {
		LOG_ERROR("ERROR_RTC_STUN invalid stun packet, size=%d", stream->size());
		return -1;
	}

	pkt->message_type = stream->read2bytes();
	uint16_t message_len = stream->read2bytes();
	char magic_cookie[4] ;
	stream->readBytes(magic_cookie,4);

	char transcation_id[12];
	stream->readBytes(transcation_id, 12);
	pkt->transcation_id = std::string(transcation_id, 12);
	if (nb_buf != 20 + message_len) {
		LOG_ERROR("ERROR_RTC_STUN invalid stun packet, message_len=%d, nb_buf=%d", message_len, nb_buf);
		return -1;
	}
	while (stream->left() >= 4) {
		uint16_t type = stream->read2bytes();
		uint16_t len = stream->read2bytes();
		if (stream->left()  < len) {
			LOG_ERROR("invalid stun packet");
			return -1;
		}

		char* val = new char[len];
		stream->readBytes(val, len);
		// padding
		if (len % 4 != 0) {
			stream->skip(4 - (len % 4));
		}

		switch (type) {
		case StunUsername: {
			pkt->username = std::string(val,len);
			char* p = strtok(val,":");
			if (p) {
				//strcpy(pkt->local_ufrag, p);
				pkt->local_ufrag = p;
				p = strtok(NULL,":");
				if (p) {
					//strcpy(pkt->remote_ufrag, p);
					pkt->remote_ufrag = p;
				}
			}
			break;
		}

		case StunUseCandidate: {
			pkt->use_candidate = true;
			//yang_trace("stun use-candidate");
			break;
		}
		// @see: https://tools.ietf.org/html/draft-ietf-ice-rfc5245bis-00#section-5.1.2
		// One agent full, one lite:  The full agent MUST take the controlling
		// role, and the lite agent MUST take the controlled role.  The full
		// agent will form check lists, run the ICE state machines, and
		// generate connectivity checks.
		case StunIceControlled: {
			pkt->ice_controlled = true;
			//yang_trace("stun ice-controlled");
			break;
		}
		case StunIceControlling: {
			pkt->ice_controlling = true;
			//yang_trace("stun ice-controlling");
			break;
		}
		default: {
			break;
		}
		}
	}

	return 0;
}

int32_t RtcStun::decode2(char* buf,  int32_t nb_buf) {
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, nb_buf);
	if (stream->left() < 20) {
		LOG_ERROR("ERROR_RTC_STUN invalid stun packet, size=%d", stream->size());
		return -1;
	}

	stream->skip(2);
	uint16_t message_len = stream->read2bytes();
	stream->skip(4);
	stream->skip(12);
	if (nb_buf != 20 + message_len) {
		LOG_ERROR("ERROR_RTC_STUN invalid stun packet, message_len=%d, nb_buf=%d", message_len, nb_buf);
		return -1;
	}

	while (stream->left() >= 4) {
		//uint16_t type = yang_read_2bytes(&stream);
		stream->skip(2);
		uint16_t len = stream->read2bytes();
		if (stream->left() < len) {
			LOG_ERROR("invalid stun packet");
			return -1;
		}

		stream->skip(len);
		// padding
		if (len % 4 != 0) {
			stream->skip(4 - (len % 4));
		}
	}

	return 0;
}

//int32_t RtcStun::createResponseStunPacket(StunPacket* request,void* session) {
//	YangRtcSession* session=(YangRtcSession*)psession;
//	char s[1024] = { 0 };
//	YangRWBuffer stream;
//	yang_init_buffer(&stream,s, 1024);
//	YangStunPacket packet;
//	yang_memset(&packet,0,sizeof(YangStunPacket));
//
//	packet.mapped_address=yang_addr_getIP(&session->context.sock->session.remote_addr);
//	packet.mapped_port=yang_addr_getPort(&session->context.sock->session.remote_addr);
//
//	yang_memcpy(packet.transcation_id,request->transcation_id,12);
//	yang_strcpy(packet.local_ufrag,session->local_ufrag);
//	yang_strcpy(packet.remote_ufrag,session->remote_ufrag);
//	yang_stun_encode_binding_response(&packet,session->localIcePwd,&stream);
//
//	return session->context.sock->write(&session->context.sock->session,stream.data, yang_buffer_pos(&stream));
//}

int32_t RtcStun::createRequestStunPacket(void* psession,char* ice_pwd) {
	char s[1024] = { 0 };
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(s, 1024);
	StunPacket packet;
	packet.message_type = StunBindingRequest;
	//strcpy(packet.local_ufrag, session->local_ufrag);
	//strcpy(packet.remote_ufrag, session->remote_ufrag);

	//RWBufferSp property_username = stun_encode_rtcusername(&packet);
	//stun_encode_binding_request(&packet, &stream, ice_pwd, property_username, property_username_len);

	return 0;
}

int32_t StunLib::encode_header(StunPacket* pkt, RWBuffer* stream, int32_t len) {
	stream->write2bytes(pkt->message_type);
	stream->write2bytes(len);
	stream->write4bytes(kStunMagicCookie);
	pkt->transcation_id = generateRandMixedStr(12);
	stream->writeBytes(pkt->transcation_id.c_str(), pkt->transcation_id.size());

	return 0;
}

RWBufferSp StunLib::encode_username(char* username) {
	char buf[1460];
	uint32_t ret=0;
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes(StunUsername);
	stream->write2bytes(strlen(username));
	stream->writeBytes(username, strlen(username));
	if (stream->pos() % 4 != 0) {
		char padding[4] = {0};
		stream->writeBytes(padding, 4 - (stream->pos() % 4));
	}
	
	return stream;
}

RWBufferSp StunLib::encode_password(char* password) {
	char buf[1460];
	uint32_t ret=0;
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes(StunPassword);
	stream->write2bytes(strlen(password));
	stream->writeBytes(password, strlen(password));
	if (stream->pos() % 4 != 0) {
		char padding[4] = {0};
		stream->writeBytes(padding, 4 - (stream->pos() % 4));
	}
	
	return stream;
}

RWBufferSp StunLib::encode_transport(uint8_t protocol) {
	char buf[1460];
	uint32_t ret = 0;
	char tmp[4] = {0};
	tmp[0] = protocol;
	
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes(StunRequestTransport);
	stream->write2bytes(4);
	stream->writeBytes(tmp, 4);

	return stream;
}

RWBufferSp StunLib::encode_data(char* data) {
	char buf[1460];
	uint32_t ret=0;
	
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes((int16_t)StunData);
	stream->write2bytes(strlen(data));
	stream->writeBytes(data, strlen(data));
	if (stream->pos() % 4 != 0) {
		char padding[4] = {0};
		stream->writeBytes(padding, 4 - (stream->pos() % 4));
	}
	
	return stream;
}

RWBufferSp StunLib::encode_realm(char* realm) {
	char buf[1460];
	uint32_t ret=0;

	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes((int16_t)StunRealm);
	stream->write2bytes(strlen(realm));
	stream->writeBytes(realm, strlen(realm));
	if (stream->pos() % 4 != 0) {
		char padding[4] = {0};
		stream->writeBytes(padding, 4 - (stream->pos() % 4));
	}
	
	return stream;
}

RWBufferSp StunLib::encode_nonce(char* nonce, uint16_t len) {
	char buf[1460];
	uint32_t ret=0;
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes((int16_t)StunNonce);
	stream->write2bytes((int16_t)len);
	stream->writeBytes(nonce,len);
	if (stream->pos() % 4 != 0) {
		char padding[4] = {0};
		stream->writeBytes(padding, 4 - (stream->pos() % 4));
	}
	
	return stream;
}

RWBufferSp StunLib::encode_lifetime(int32_t lifetime) {
	char buf[1460];
	uint32_t ret=0;

	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes((int16_t)StunLifetime);
	stream->write2bytes(4);
	stream->write4bytes(lifetime);

	return stream;
}

RWBufferSp StunLib::encode_channelNumber(uint16_t channelNum) {
	char buf[1460];
	uint32_t ret=0;
	uint16_t reserve=0;
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes((int16_t)StunChannelNumber);
	stream->write2bytes(4);
	stream->write2bytes(channelNum);
	stream->write2bytes(reserve);

	return stream;
}

RWBufferSp StunLib::encode_peer_address_ipv4(uint32_t address, uint16_t port) {
	char buf[1460];
	uint32_t ret=0;
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes(StunXorPeerAddress);
	stream->write2bytes(8);
	stream->write1bytes(0); // ignore this bytes
	stream->write1bytes(1); // ipv4 family
	stream->write2bytes(port ^ (kStunMagicCookie >> 16));
	stream->write4bytes(address ^ kStunMagicCookie);

	return stream;
}

RWBufferSp StunLib::encode_hmac(RWBuffer *pstream, char* pwd) {
	uint32_t ret=0;
	char buf[1460];
	char hmac_buf[20] = {0};
	uint32_t hmac_buf_len = 0;
	if (hmac_encode("sha1", pwd, strlen(pwd), pstream->data(), pstream->pos(), hmac_buf, &hmac_buf_len) < 0) {
		return nullptr;
	}

	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes(StunMessageIntegrity);
	stream->write2bytes(hmac_buf_len);
	stream->writeBytes(hmac_buf, hmac_buf_len);
	if (stream->pos() % 4 != 0) {
		char padding[4] = {0};
		stream->writeBytes(padding, 4 - (stream->pos() % 4));
	}

	return stream;
}

RWBufferSp StunLib::encode_fingerprint(RWBuffer *pstream) {
	char buf[1460];
	uint32_t ret = 0;
	//Fingerprint
	uint32_t crc32 = crc32_ieee(pstream->data(), pstream->pos(), 0)^ 0x5354554E;

	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes((int16_t)StunFingerprint);
	stream->write2bytes(4);
	stream->write4bytes(crc32);

	return stream;
}

RWBufferSp StunLib::encode_mapped_address_ipv4(StunPacket* pkt) {
	char buf[1460];
	uint32_t ret=0;
	RWBufferSp stream = std::make_shared<RWBuffer>();
	stream->init(buf, sizeof(buf));
	stream->write2bytes(StunXorMappedAddress);
	stream->write2bytes(8);
	stream->write1bytes(0); // ignore this bytes
	stream->write1bytes(1); // ipv4 family
	stream->write2bytes(pkt->mapped_port ^ (kStunMagicCookie >> 16));
	stream->write4bytes(pkt->mapped_address ^ kStunMagicCookie);
	
	return stream;
}

int32_t StunLib::encode_request(StunMessageType type, RWBuffer* stream, SessionInfo remote, SessionInfo local) {
	StunPacket packet;
	packet.message_type = type;
	packet.transcation_id = generateRandMixedStr(12);
	packet.remote_ufrag = remote.iceUfrag;
	packet.local_ufrag = local.iceUfrag;

	RWBufferSp property_username = stun_encode_rtcusername(&packet);
	stun_encode_binding_request2(&packet, stream, remote.icePwd.c_str(), property_username->data(), property_username->pos());

	return 0;
}

}