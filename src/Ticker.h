#ifndef FRTC_TICKER_H
#define FRTC_TICKER_H

#include <cstdint>
#include <memory>
#include "Function.h"

namespace frtc {

class Ticker {
public:
    Ticker() {
        _beginTs = getCurrentMillisecond();
        _createTs = getCurrentMillisecond();
    }

    /**
     * 获取上次resetTime后至今的时间，单位毫秒
     */
    uint64_t elapsedTime() const {
        return getCurrentMillisecond() - _beginTs;
    }

    /**
     * 获取从创建至今的时间，单位毫秒
     */
    uint64_t createdTime() const {
        return getCurrentMillisecond() - _createTs;
    }

    /**
     * 重置计时器
     */
    void resetTime() {
        _beginTs = getCurrentMillisecond();
    }

private:
    uint64_t _beginTs;
    uint64_t _createTs;
};

typedef std::shared_ptr<Ticker> TickerSp;

}

#endif