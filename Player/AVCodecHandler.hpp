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
#include "QueueDef.h"

extern "C" {
  #include <libavformat/avformat.h>
  #include <libavcodec/avcodec.h>

  #include <libswscale/swscale.h>
  #include <libavutil/avutil.h>
}

enum MediaType {
  MediaTypeVideo = 0,
  MediaType
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
  
  void        setVideoFilePath(const std::string &path);
  std::string getVideoFilePath();
  int         getVideoWidth();
  int         getVideoHeight();
  
  int         initVideoCodec();
  int         destoryVideoCodec();
  
  void        startPlayVideo();
  void        stopPlayVideo();
  
private:
  void doReadMediaFrameThread();
  void doAudioDecodePlayThread();
  void doVideoDecodePlayThread();
  
  void startMediaProcessThreads();
  void waitAllTreadsExit();
  
  void stdThreadSleep(int mseconds);
private:
  int         m_videoWidth = 0;
  int         m_videoHeight = 0;
  std::string m_videoPathString = "";
  /*
   文件的根所有流媒体文件都要通过它来处理, 所有视频信息的发源地
   两条流：
    音频：
    视频：
   */
  AVFormatContext *m_pformatCtx = NULL;
  int             m_videoStreamIndex = -1;// why not 0?
  int             m_audioStreamIndex = -1;
  
  AVCodecContext  *m_pVideoCodecCtx = NULL;
  AVCodecContext  *m_pAudioCodecCtx = NULL;
  
  // 时间基
  AVRational      m_vStreamTimeRational;
  AVRational      m_aStreamTimeRational;
  
  MediaQueue<AVPacket *> m_audioPacketQueue;
  MediaQueue<AVPacket *> m_videoPacketQueue;
  
  MediaPlayStatus m_eMediaPlayStatus;
  bool            m_bReadFileEOF = false;
  
  
  void     readMediaPacket();
  void     freePacket(AVPacket* pkt);
  
};









#endif /* AVCodecHandler_hpp */
