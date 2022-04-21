//
//  AVCodecHandler.hpp
//  Player
//
//  Created by 常仲伟 on 2022/4/21.
//

#ifndef AVCodecHandler_hpp
#define AVCodecHandler_hpp

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

#include <stdio.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
}

enum MediaType {
  MediaTypeVideo = 0,
  MediaTypeAudio
};

enum MediaPlayStatus {
  MediaPlayStatusPlaying = 0,
  MediaPlayStatusPause,
  MediaPlayStatusSeek,
  MediaPlayStatusStop
};

class AVCodecHandler {
  
public:
  AVCodecHandler();
  ~AVCodecHandler();
  
  void setVideoFilePath(const std::string &path);
private:
  
};









#endif /* AVCodecHandler_hpp */
