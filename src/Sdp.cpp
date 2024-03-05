#include <sstream>
#include <iostream>
#include <cstring>
#include "Sdp.h"
#include "Function.h" 

namespace frtc {

std::string Sdp::dump() {
    std::stringstream sdp;
    sdp << "v=" << version  << "\r\n"
        << "o=" << "- " << sessionId << " " << sessionVersion << " " << netType << " " << addrType << " " << unicastAddress << " \r\n"
        << "s=" << "-\r\n"
        << "t=" << startTime << " " << endTime << "\r\n"
        << "a=group:BUNDLE " << videoDesc.mid << " " << audioDesc.mid << "\r\n"
        << "m=" << videoDesc.type << " " << videoDesc.port << " " << videoDesc.protos << mediaTypeLst(videoDesc) << "\r\n"
        << "c=" << netType << " " << addrType << " " << unicastAddress << "\r\n"
        << "a=rtcp:" << videoDesc.port << " " << netType << " " << addrType << " " << unicastAddress << "\r\n" 
        << "a=ice-ufrag:" << videoDesc.sessionInfo.iceUfrag << "\r\n"
        << "a=ice-pwd:" << videoDesc.sessionInfo.icePwd << "\r\n" 
        << "a=ice-options:" << videoDesc.sessionInfo.iceOptions << "\r\n"
        << "a=fingerprint:" << videoDesc.sessionInfo.fingerprintAlgo << " " << videoDesc.sessionInfo.fingerprint << "\r\n"
        << "a=setup:" << videoDesc.sessionInfo.setup << "\r\n"
        << "a=mid:" << videoDesc.mid << "\r\n"
        << "extmap:1 " << gExtMap[0] << "\r\n"
        << "extmap:2 " << gExtMap[1] << "\r\n"
        << "extmap:3 " << gExtMap[2] << "\r\n"
        << "extmap:4 " << gExtMap[3] << "\r\n"
        << "extmap:5 " << gExtMap[4] << "\r\n"
        << "extmap:6 " << gExtMap[5] << "\r\n"
        << "extmap:7 " << gExtMap[6] << "\r\n"
        << "extmap:8 " << gExtMap[7] << "\r\n"
        << "extmap:9 " << gExtMap[8] << "\r\n"
        << "extmap:10 " << gExtMap[9] << "\r\n"
        << "extmap:11 " << gExtMap[10] << "\r\n"
        << "a=" << videoDesc.direct << "\r\n"
        << "a=" << videoDesc.mux << "\r\n"
        << "a=" << videoDesc.rsize << "\r\n"
        << payloadInfoLst(videoDesc)
        << "m=" << audioDesc.type << " " << audioDesc.port << " " << audioDesc.protos << mediaTypeLst(audioDesc) << "\r\n" 
        << "c=" << netType << " " << addrType << " " << unicastAddress << "\r\n"
        << "a=rtcp:" << audioDesc.port << " " << netType << " " << addrType << " " << unicastAddress << "\r\n" 
        << "a=ice-ufrag:" << audioDesc.sessionInfo.iceUfrag << "\r\n"
        << "a=ice-pwd:" << audioDesc.sessionInfo.icePwd << "\r\n" 
        << "a=ice-options:" << audioDesc.sessionInfo.iceOptions << "\r\n"
        << "a=fingerprint:" << audioDesc.sessionInfo.fingerprintAlgo << " " << audioDesc.sessionInfo.fingerprint << "\r\n"
        << "a=setup:" << audioDesc.sessionInfo.setup << "\r\n"
        << "a=mid:" << audioDesc.mid << "\r\n"
        << "a=extmap:14 " << gExtMap[11] << "\r\n" 
        << "a=extmap:2 " << gExtMap[1] << "\r\n" 
        << "a=extmap:4 " << gExtMap[3] << "\r\n" 
        << "a=extmap:9 " << gExtMap[8] << "\r\n" 
        << "a=" << audioDesc.direct << "\r\n"  
        << "a=" << audioDesc.mux << "\r\n"  
        << "a=" << audioDesc.rsize << "\r\n"
        << payloadInfoLst(audioDesc);

    return sdp.str();
}
	
std::string Sdp::create(const std::string& fingerprint) {
    startTime = 0;
    endTime = 0;
    version = "0";
    userName = "frtc";
    sessionId = generateRandDigit(19);
    sessionVersion = "2";
    netType = "IN";
    addrType = "IP4";
    unicastAddress = "0.0.0.0";
    sessionName = "frtcSession";
    // video
    videoDesc.type = "video";
    videoDesc.port = 9;
    videoDesc.protos = "UDP/TLS/RTP/SAVPF";
    videoDesc.direct = "recvonly";
    videoDesc.mux = "rtcp-mux";
    videoDesc.rsize = "rtcp-rsize";
    videoDesc.mid = "0";
    videoDesc.sessionInfo.iceUfrag = generateRandStr(4); 
    videoDesc.sessionInfo.icePwd = generateRandStr(32); 
    videoDesc.sessionInfo.iceOptions = "trickle";
    videoDesc.sessionInfo.fingerprintAlgo = "sha-256"; 
    videoDesc.sessionInfo.fingerprint = fingerprint; 
    videoDesc.sessionInfo.setup = "active"; 
    videoDesc.payloads.push_back(MediaPayload());
    videoDesc.payloads[0].payloadType = 96; 
    videoDesc.payloads[0].clockRate = 90000;
    videoDesc.payloads[0].encodingName = "h264";
    videoDesc.payloads[0].formatSpecificParam = "level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f";
    videoDesc.payloads[0].rtcpFbs.push_back("goog-remb");
    videoDesc.payloads[0].rtcpFbs.push_back("transport-cc");
    videoDesc.payloads[0].rtcpFbs.push_back("ccm fir");
    videoDesc.payloads[0].rtcpFbs.push_back("nack");
    videoDesc.payloads[0].rtcpFbs.push_back("nack pli");
    videoDesc.payloads.push_back(MediaPayload());
    videoDesc.payloads[1].payloadType = 97; 
    videoDesc.payloads[1].clockRate = 90000;
    videoDesc.payloads[1].encodingName = "h265";
    videoDesc.payloads[1].formatSpecificParam = "level-asymmetry-allowed=1;packetization-mode=1;profile-level-id=42001f";
    videoDesc.payloads[1].rtcpFbs.push_back("goog-remb");
    videoDesc.payloads[1].rtcpFbs.push_back("transport-cc");
    videoDesc.payloads[1].rtcpFbs.push_back("ccm fir");
    videoDesc.payloads[1].rtcpFbs.push_back("nack");
    videoDesc.payloads[1].rtcpFbs.push_back("nack pli");
    // audio
    audioDesc.type = "audio";
    audioDesc.port = 9;
    audioDesc.protos = "UDP/TLS/RTP/SAVPF";
    audioDesc.direct = "recvonly";
    audioDesc.mux = "rtcp-mux";
    audioDesc.rsize = "rtcp-rsize";
    audioDesc.mid = "1";
    audioDesc.sessionInfo = videoDesc.sessionInfo;
    audioDesc.payloads.push_back(MediaPayload());
    audioDesc.payloads[0].payloadType = 99; 
    audioDesc.payloads[0].clockRate = 8000;
    audioDesc.payloads[0].encodingName = "PCMA";
    audioDesc.payloads.push_back(MediaPayload());
    audioDesc.payloads[1].payloadType = 100; 
    audioDesc.payloads[1].clockRate = 8000;
    audioDesc.payloads[1].encodingName = "PCMU";

    return dump();
}

void Sdp::parse(const std::string& sdp) {
    std::vector<std::string> lines = splitStrWithSeparator(sdp, "\r\n");
    for (auto iter = lines.begin(); iter != lines.end();) {
        std::vector<std::string> words = splitStrWithSeparator(*iter, "=");
        if (words.size() <= 1) {
            iter++;
            continue;
        }
        if (words[0] == "v") {
            version = words[1];
        
        } else if (words[0] == "o") {
            std::vector<std::string> values = splitStrWithSeparator(words[1], " ");
            if (values.size() == 6) {
                userName = values[0];
                sessionId = values[1];
                sessionVersion = values[2];
                netType = values[3];
                addrType = values[4];
                unicastAddress = values[5];
            }

        } else if (words[0] == "s") {
            sessionName = words[1];

        } else if (words[0] == "t") {
            std::vector<std::string> values = splitStrWithSeparator(words[1], " ");
            if (values.size() == 2) {
                startTime = std::atoi(values[0].c_str());
                startTime = std::atoi(values[0].c_str());
            }

        } else if (words[0] == "a") {
            if (words[1].find("group") != std::string::npos) {

            } else if (words[1].find("msid") != std::string::npos) {

            }

        } else if (words[0] == "m") {
            break;
        }
    
        iter = lines.erase(iter);
    }

    std::vector<std::string> videoAttribute;
    std::vector<std::string> audioAttribute;
    bool video = false;
    for (auto& line : lines) {
        std::vector<std::string> words = splitStrWithSeparator(line, "=");
        if (words.size() <= 1) {
            continue;
        }
        if (words[0] == "m") {
            if (words[1].find("video") != std::string::npos) {
                video = true;
            } else {
                video = false;
            }
        }
        if (video) {
            videoAttribute.push_back(line);
        } else {
            audioAttribute.push_back(line);
        }
    }
    for (auto& attribute : videoAttribute) {
        std::vector<std::string> words = splitStrWithSeparator(attribute, "=");
        if (words.size() <= 1) {
            continue;
        }
        if (words[0] == "m") {
            std::vector<std::string> values = splitStrWithSeparator(attribute, " ");
            if (values.size() >= 3) {
                videoDesc.type = values[0];
                videoDesc.port = std::atoi(values[1].c_str());        
                videoDesc.protos = values[2];
            
                for (auto iter = values.begin() + 3; iter != values.end(); iter++) {
                    MediaPayload payload;
                    payload.payloadType = std::atoi((*iter).c_str());

                    videoDesc.payloads.push_back(payload);
                }
            } 

        } else if (words[0] == "c") {
            std::vector<std::string> values = splitStrWithSeparator(attribute, " ");
            if (values.size() == 3) {
                netType = values[0];
                addrType = values[1];
                unicastAddress = values[2];
            }

        } else if (words[0] == "a") {
            if (words[1].find("ice-ufrag:") == 0) {
                videoDesc.sessionInfo.iceUfrag = words[1].c_str() + strlen("ice-ufrag:");

            } else if (words[1].find("ice-pwd:") == 0) {
                videoDesc.sessionInfo.icePwd = words[1].c_str() + strlen("ice-pwd:");

            } else if (words[1].find("ice-options:") == 0) {
                videoDesc.sessionInfo.iceOptions = words[1].c_str() + strlen("ice-options:");

            } else if (words[1].find("fingerprint:") == 0) {
                std::vector<std::string> values = splitStrWithSeparator(words[1], " ");
                if (values.size() == 2) {
                    videoDesc.sessionInfo.fingerprintAlgo = values[0].c_str() + strlen("fingerprint:");
                    videoDesc.sessionInfo.fingerprint = values[1];
                }
            
            } else if (words[1].find("setup:") == 0) {
                videoDesc.sessionInfo.setup = words[1].c_str() + strlen("setup:");

            } else if (words[1].find("mid:") == 0) {
                videoDesc.mid = words[1].c_str() + strlen("mid:");

            } else if (words[1].find("extmap:") == 0) {
                std::vector<std::string> values = splitStrWithSeparator(words[1], " ");
                if (values.size() == 2) {
                    int32_t index = std::atoi(values[0].c_str() + strlen("extmap:"));
                    videoDesc.extmap[index] = values[1];
                }

            } else if (words[1].find("rtpmap:") == 0) {
                std::vector<std::string> values = splitStrWithSeparator(words[1], " ");
                if (values.size() == 2) {
                    int payloadType = std::atoi(values[0].c_str() + strlen("rtpmap:")); 
                    for (auto& item : videoDesc.payloads) {
                        if (item.payloadType == payloadType) {
                            std::vector<std::string> params = splitStrWithSeparator(values[1], "/");
                            if (params.size() == 2) {
                                item.encodingName = params[0];
                                item.clockRate = std::atoi(params[1].c_str());
                            }
                        }
                    }
                }

            } else if (words[1].find("rtcp-fb:") == 0) {
                std::vector<std::string> values = splitStrWithSeparator(words[1], " ");
                if (values.size() >= 1) {
                    int payloadType = std::atoi(values[0].c_str() + strlen("rtcp-fb:")); 
                    for (auto& item : videoDesc.payloads) {
                        if (item.payloadType == payloadType) {
                            for (auto iter = values.begin() + 1; iter != values.end(); iter++) {
                                item.rtcpFbs.push_back(*iter);
                            }
                        }
                    }
                }

            } else if (words[1].find("fmtp:") == 0) {
                std::vector<std::string> values = splitStrWithSeparator(words[1], " ");
                if (values.size() >= 1) {
                    int payloadType = std::atoi(values[0].c_str() + strlen("fmtp:")); 
                    for (auto& item : videoDesc.payloads) {
                        if (item.payloadType == payloadType) {
                            item.formatSpecificParam = values[1];
                        }
                    }
                }
        
            } else if (words[1].find("rtcp:") == 0) {
                std::vector<std::string> values = splitStrWithSeparator(words[1], " ");
                if (values.size() == 4) {
                    netType = values[1];
                    addrType = values[2];
                    unicastAddress = values[3];
                }
        
            } else if (words[1].find("ice-lite") == 0) {
                videoDesc.iceMode = "ice-lite";

            } else if (words[1].find("sendonly") == 0) {
                videoDesc.direct = "sendonly";

            } else if (words[1].find("sendrecv") == 0) {
                videoDesc.direct = "sendrecv";

            } else if (words[1].find("recvonly") == 0) {
                videoDesc.direct = "recvonly";
            
            } else if (words[1].find("rtcp-mux") == 0) {
                videoDesc.mux = "rtcp-mux";

            } else if (words[1].find("msid:") == 0) {

            } else if (words[1].find("ssrc:") == 0) {

            } else if (words[1].find("ssrc-group:") == 0) {

            } else if (words[1].find("candidate:") == 0) {
                std::vector<std::string> valuse = splitStrWithSeparator(words[1], " ");
                if (valuse.size() >= 8) {
                    videoDesc.candidate.protocol = valuse[2];
                    videoDesc.candidate.type = valuse[7];
                    videoDesc.candidate.ip = valuse[4];
                    videoDesc.candidate.port = std::atoi(valuse[5].c_str());
                }
            }
        }
    }
    for (auto& attribute : audioAttribute) {
        std::vector<std::string> words = splitStrWithSeparator(attribute, "=");
        if (words.size() <= 1) {
            continue;
        }
        if (words[0] == "m") {

        } else if (words[0] == "c") {

        } else if (words[0] == "a") {

        }
    }
}
	
std::string Sdp::mediaTypeLst(const MediaDesc& media) {
    std::stringstream ss;
    for (auto& item : media.payloads) {
        ss << " " << item.payloadType;
    }

    return ss.str();
}	

std::string Sdp::payloadInfoLst(const MediaDesc& media) {
    std::stringstream ss;
    for (auto& item : media.payloads) {
        ss << "a=rtpmap:" << item.payloadType << " " << item.encodingName << "/" << item.clockRate << "\r\n";
        if (media.type == "video") {
            for (auto& iter : item.rtcpFbs) {
               ss << "a=rtcp-fb:" << item.payloadType << " " << iter << "\r\n"; 
            }   
        }
    }

    return ss.str();
} 

}