#include "Stamp.h"

namespace frtc {

bool DtsGenerator::getDts(uint64_t pts, uint64_t& dts) {
    bool ret = false;
    if (pts == _last_pts) {
        // pts未变，说明dts也不会变，返回上次dts
        if (_last_dts) {
            dts = _last_dts;
            ret = true;
        }
    } else {
        // pts变了，尝试计算dts
        ret = getDts_l(pts, dts);
        if (ret) {
            // 获取到了dts，保存本次结果
            _last_dts = dts;
        }
    }

    if (!ret) {
        // pts排序列队长度还不知道，也就是不知道有没有B帧，
        // 那么先强制dts == pts，这样可能导致有B帧的情况下，起始画面有几帧回退
        dts = pts;
    }

    // 记录上次pts
    _last_pts = pts;
    return ret;
}

// 该算法核心思想是对pts进行排序，排序好的pts就是dts。
// 排序有一定的滞后性，那么需要加上排序导致的时间戳偏移量
bool DtsGenerator::getDts_l(uint64_t pts, uint64_t& dts) {
    if (_sorter_max_size == 1) {
        // 没有B帧，dts就等于pts
        dts = pts;
        return true;
    }

    if (!_sorter_max_size) {
        // 尚未计算出pts排序列队长度(也就是P帧间B帧个数)
        if (pts > _last_max_pts) {
            // pts时间戳增加了，那么说明这帧画面不是B帧(说明是P帧或关键帧)
            if (_frames_since_last_max_pts && _count_sorter_max_size++ > 0) {
                // 已经出现多次非B帧的情况，那么我们就能知道P帧间B帧的个数
                _sorter_max_size = _frames_since_last_max_pts;
                // 我们记录P帧间时间间隔(也就是多个B帧时间戳增量累计)
                _dts_pts_offset = (pts - _last_max_pts);
                // 除以2，防止dts大于pts
                _dts_pts_offset /= 2;
            }
            // 遇到P帧或关键帧，连续B帧计数清零
            _frames_since_last_max_pts = 0;
            // 记录上次非B帧的pts时间戳(同时也是dts)，用于统计连续B帧时间戳增量
            _last_max_pts = pts;
        }
        // 如果pts时间戳小于上一个P帧，那么断定这个是B帧,我们记录B帧连续个数
        ++_frames_since_last_max_pts;
    }

    // pts放入排序缓存列队，缓存列队最大等于连续B帧个数
    _pts_sorter.emplace(pts);

    if (_sorter_max_size && _pts_sorter.size() > _sorter_max_size) {
        // 如果启用了pts排序(意味着存在B帧)，并且pts排序缓存列队长度大于连续B帧个数，
        // 意味着后续的pts都会比最早的pts大，那么说明可以取出最早的pts了，这个pts将当做该帧的dts基准
        auto it = _pts_sorter.begin();

        // 由于该pts是前面偏移了个_sorter_max_size帧的pts(也就是那帧画面的dts),
        // 那么我们加上时间戳偏移量，基本等于该帧的dts
        dts = *it + _dts_pts_offset;
        if (dts > pts) {
            // dts不能大于pts(基本不可能到达这个逻辑)
            dts = pts;
        }

        // pts排序缓存出列
        _pts_sorter.erase(it);
        return true;
    }

    // 排序缓存尚未满
    return false;
}

}