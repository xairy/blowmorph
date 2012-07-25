#ifndef BM_INTERPOLATOR_HPP_
#define BM_INTERPOLATOR_HPP_

#include <list>

namespace interpolator {

template<class V> V lerp(const V& a, const V& b, double bRatio) {
  return V(a * (1 - bRatio) + b * bRatio);
}

template<class FrameT, class TimeT>
class LinearInterpolator {
public:
  typedef std::pair<FrameT, TimeT> TimedFrame;

  LinearInterpolator(TimeT timeOffset, size_t frameCount = 2)
    : timeOffset(timeOffset), frameCount(frameCount) { }

  size_t GetFrameCount() const {
    return frameCount;
  }
  void SetFrameCount(size_t count) {
    frameCount = count;
    while (frames.size() > frameCount) {
      frames.pop_front();
    }
  }
  
  TimeT GetTimeOffset() const {
    return timeOffset;
  }
  void SetTimeOffset(TimeT value) {
    timeOffset = value;
  }

  void Push(const FrameT& frame, TimeT time) {
    // drop frame if it is too late / time is the same
    if (frames.size() > 0) {
      if (time <= frames.back().second) {
        return;
      }
    }
    
    // add frame to the frame list, dropping previous if neccessary
    if (frames.size() == frameCount) {
      frames.pop_front();
    }
    frames.push_back(TimedFrame(frame, time));
  }
  
  void Clear() {
    frames.clear();
  }

  FrameT Interpolate(TimeT time) {
    // if we don't have enough frames to interpolate - return default value
    if (frames.size() == 0) {
      return FrameT();
    } else if (frames.size() == 1) {
      return frames.back().first;
    }
    
    // subtract lag time (i.e. the difference in times on server and client)
    time -= timeOffset;

    // get the last two frames
    std::list<TimedFrame>::reverse_iterator it = frames.rbegin();
    TimedFrame frame2 = *it;
    TimedFrame frame1 = *(++it);
    
    double ratio = ((double) time - (double)frame1.second) / ((double) frame2.second - (double) frame1.second);
    
    // interpolate over them
    return lerp(frame1.first, frame2.first, ratio);
  }

private:
  size_t frameCount;
  std::list<TimedFrame> frames;
  TimeT timeOffset;
};

/*struct Frame {
  glm::vec2 position;
};
template<> Frame lerp(Frame& a, Frame& b, double bRatio) {
  Frame result;
  result.position = a.position * (1 - bRatio) + b.position * bRatio;
  return result;
}

typedef LinearInterpolator<Frame, bm::uint32_t> Interpolator;*/

}; // namespace interpolator

#endif//BM_INTERPOLATOR_HPP_