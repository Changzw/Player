//
//  AVCodecHandler.cpp
//  Player
//
//  Created by 常仲伟 on 2022/4/21.
//

#include "AVCodecHandler.hpp"
#include "QueueDef.h"
#include "YUVDataDefine.h"

std::atomic<bool> m_bThreadRunning(false);// 在多线程中是原子操作

AVCodecHandler::AVCodecHandler() {
//  avcodec_register_all();
  av_register_all();
  
  const char *config = avcodec_configuration();
  printf("%d\n, %s\n",  avcodec_version(), config);
  printf("----------------------\n");
}

AVCodecHandler::~AVCodecHandler() {
  
}

void AVCodecHandler::setVideoFilePath(const std::string &path){
  m_videoPathString = path;
}

std::string AVCodecHandler::getVideoFilePath(){
  return m_videoPathString;
}
int AVCodecHandler::getVideoWidth(){
  return m_videoWidth;
  
}
int AVCodecHandler::getVideoHeight(){
  return m_videoHeight;
}

int AVCodecHandler::initVideoCodec(){
  if (m_videoPathString == "") {
    return  -1;
  }
  const char* filPath = m_videoPathString.c_str();
  if (avformat_open_input(&m_pformatCtx, filPath, NULL, NULL) != 0) {
    printf("avformat open input failed...\n");
    return -1;
  }
  
  if (avformat_find_stream_info(m_pformatCtx, NULL) < 0) {
    printf("avformat find stream failed...\n");
    return -1;
  }
  
  av_dump_format(m_pformatCtx, 0, filPath, 0);
  printf("---------------------------\n");
  
  m_videoStreamIndex = -1;
  m_audioStreamIndex = -1;
  for (int i=0; i<(int)m_pformatCtx->nb_streams; i++){
    AVCodecParameters *codecParamters = m_pformatCtx->streams[i]->codecpar;
    if (codecParamters->codec_type == AVMEDIA_TYPE_VIDEO) {
      m_videoStreamIndex = i;
      AVCodec *codec = avcodec_find_decoder(codecParamters->codec_id);// 解码信息
      if (codec == NULL) {
        printf("avcodec_find_decoder failed...\n");
        return -1;
      }
      m_pVideoCodecCtx = avcodec_alloc_context3(codec);// 申请解码器上下文
      avcodec_parameters_to_context(m_pVideoCodecCtx, codecParamters);//copy param to context
      if (avcodec_open2(m_pVideoCodecCtx, codec, NULL) < 0) {
        printf("v: avcodec_open2 video failed...\n");
        return -1;
      }
    }else if (codecParamters->codec_type == AVMEDIA_TYPE_AUDIO) {
      m_audioStreamIndex = i;
      AVCodec *codec = avcodec_find_decoder(codecParamters->codec_id);// 解码信息
      if (codec == NULL) {
        printf("avcodec_find_decoder failed...\n");
        return -1;
      }
      m_pAudioCodecCtx = avcodec_alloc_context3(codec);// 申请解码器上下文
      avcodec_parameters_to_context(m_pAudioCodecCtx, codecParamters);//copy param to context
      if (avcodec_open2(m_pAudioCodecCtx, codec, NULL) < 0) {
        printf("a: avcodec_open2 audio failed...\n");
        return -1;
      }
    }
  }
  
  m_videoWidth = m_pVideoCodecCtx->width;
  m_videoHeight = m_pVideoCodecCtx->height;
  
  /*
   时间基，用于编解码，使用 AVRational类型保存
   */
  AVStream *videoStream = m_pformatCtx->streams[m_videoStreamIndex];
  AVStream *audioStream = m_pformatCtx->streams[m_audioStreamIndex];
  m_vStreamTimeRational = videoStream->time_base;
  m_aStreamTimeRational = audioStream->time_base;
  printf("V Time base num: %d, den: %d\n", m_vStreamTimeRational.num, m_vStreamTimeRational.den);
  printf("A Time base num: %d, den: %d\n", m_aStreamTimeRational.num, m_aStreamTimeRational.den);
  
  printf("Stream IDX: Video: %d, Audio: %d\n", m_videoStreamIndex, m_audioStreamIndex);
  printf("Stream Size: Height: %d, Width: %d\n", m_videoHeight, m_videoWidth);
  return 0;
}

int AVCodecHandler::destoryVideoCodec(){
  
  return 0;
}

void AVCodecHandler::startPlayVideo() {
  
}

void AVCodecHandler::stopPlayVideo(){
  
}

void AVCodecHandler::startMediaProcessThreads() {
  m_bThreadRunning = true;
  // C++ 11 线程
  std::thread readThread(&AVCodecHandler::doReadMediaFrameThread,this);
  readThread.detach();
  
  std::thread audioThread(&AVCodecHandler::doAudioDecodePlayThread,this);
  audioThread.detach();
  
  std::thread videoThread(&AVCodecHandler::doVideoDecodePlayThread,this);
  videoThread.detach();
}

void AVCodecHandler::waitAllTreadsExit(){
  
}

void AVCodecHandler::stdThreadSleep(int mseconds){
  std::chrono::milliseconds sleepTime(mseconds);
  std::this_thread::sleep_for(sleepTime);
}

void AVCodecHandler::readMediaPacket() {
  AVPacket *packet = (AVPacket*)malloc(sizeof(AVPacket));
  if (NULL == packet) {
    return;
  }
  av_init_packet(packet);
  m_eMediaPlayStatus = MediaPlayStatusPlaying;
  int retValue = av_read_frame(m_pformatCtx, packet);
  if (retValue == 0) {
    if (packet->stream_index == m_videoStreamIndex) {
      if (!av_dup_packet(packet)) {// 这个是啥？
        m_videoPacketQueue.enqueue(packet);
      }else {
        freePacket(packet);
      }
    } else if (packet->stream_index == m_audioStreamIndex) {
      if (!av_dup_packet(packet)) {
        m_audioPacketQueue.enqueue(packet);
      }else {
        freePacket(packet);
      }
    }
  }else if (retValue < 0) {
    if ((m_bReadFileEOF == false) && (retValue == AVERROR_EOF)) {
      m_bReadFileEOF = true;
    }
  }
}

void AVCodecHandler::doReadMediaFrameThread() {
  while (m_bThreadRunning) {
    /*
     read control一定要控制着读，要不然一下子就会被读完，你的内存会暴涨
     */
    m_bThreadRunning = true;
    if (m_eMediaPlayStatus == MediaPlayStatusPause) {
      stdThreadSleep(10);// 让 thread放弃 cpu 资源
      continue;
    }
    if (m_videoPacketQueue.size() > MAX_VIDEO_FRAME_IN_QUEUE && m_audioPacketQueue.size() > MAX_AUDIO_FRAME_IN_QUEUE) {
      stdThreadSleep(10);
      continue;
    }
    
    if (m_bReadFileEOF == false) {
      readMediaPacket();
    }else {
      stdThreadSleep(10);// 当文件读完以后，还有几帧需要解码，队列里面还有数据
    }
    
  }
  
  m_bThreadRunning = true;
}

void AVCodecHandler::doAudioDecodePlayThread(){
  
}
void AVCodecHandler::doVideoDecodePlayThread(){
  
}

void AVCodecHandler::freePacket(AVPacket* pkt) {
  if(pkt == NULL ){
    return;
  }
  av_free_packet(pkt);
  free(pkt);
}
