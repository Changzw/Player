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
#include "YUVDataDefine.h"

#ifndef int64
#define int64 long long
#endif

extern "C" {
  #include <libavformat/avformat.h>
  #include <libavcodec/avcodec.h>
  #include <libswresample/swresample.h>
  #include <libswscale/swscale.h>
  #include <libavutil/avutil.h>
}

typedef void  (*UpdateVideo2GUI_Callback)    (YUVData_Frame* yuv,unsigned long userData);
typedef void  (*UpdateAudio2GUI_Callback)    (unsigned char* pcmBuff,unsigned int size ,unsigned long userData);
typedef void  (*UpdateCurrentPTS_Callback)   (float pts,unsigned long userData);

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
  void     convertAndRenderVideo(AVFrame* decodedFrame,long long ppts);
  void     convertAndPlayAudio(AVFrame* decodedFrame);
  void     copyDecodedFrame(uint8_t* src, uint8_t* dist,int linesize, int width, int height);
  void     copyDecodedFrame420(uint8_t* src, uint8_t* dist,int linesize, int width, int height);

  void  tickVideoFrameTimerDelay(int64_t pts);
  void  tickAudioFrameTimerDelay(int64_t pts);
  
  void doReadMediaFrameThread();
  void doAudioDecodePlayThread();
  void doVideoDecodePlayThread();
  
  void startMediaProcessThreads();
  void waitAllTreadsExit();
  
  void stdThreadSleep(int mseconds);
  
  void     readMediaPacket();
  void     freePacket(AVPacket* pkt);
  void     resetAllMediaPlayerParameters();
  
  /*
   用音频同步视频，因为音频是根据 44100 2 16，三个参数播的，而帧组，帧组内的帧是不确定的
   */
  float    getAudioTimestampFramPTS(int64_t pts);
  float    getVideoTimestampFramPTS(int64_t pts);// 让音频等视频
  
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
  AVFormatContext *m_pFormatCtx = NULL;
  int             m_videoStreamIdx = -1;// why not 0?
  int             m_audioStreamIdx = -1;
  
  AVCodecContext  *m_pVideoCodecCtx = NULL;
  AVCodecContext  *m_pAudioCodecCtx = NULL;
  
  SwrContext      *m_pAudioSwrCtx = NULL;
  SwsContext      *m_pVideoSwsCtx = NULL;

  // 时间基
  AVRational      m_vStreamTimeRational;
  AVRational      m_aStreamTimeRational;
  
  // 编码了的数据 ACC，H.264
  MediaQueue<AVPacket *> m_audioPacketQueue;
  MediaQueue<AVPacket *> m_videoPacketQueue;
  
  // 解码后的数据 YUV，PCM
  AVFrame         *m_pVideoFrame = NULL;
  AVFrame         *m_pAudioFrame = NULL;
  
  AVFrame         *m_pYUVFrame   = NULL;
  
  unsigned long               m_userDataVideo=0;
  UpdateVideo2GUI_Callback    m_updateVideoCallback = NULL;
  
  unsigned long               m_userDataAudio=0;
  UpdateAudio2GUI_Callback    m_updateAudioCallback = NULL;
  
  unsigned long               m_userDataPts=0;
  UpdateCurrentPTS_Callback   m_updateCurrentPTSCallback = NULL;

  MediaPlayStatus m_eMediaPlayStatus;
  bool            m_bReadFileEOF = false;
  
  float           m_nCurrentAudioTimestamp  = 0.0f;
  float           m_nLastAudioTimestamp     = 0.0f;

};









#endif /* AVCodecHandler_hpp */
