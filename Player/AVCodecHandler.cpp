//
//  AVCodecHandler.cpp
//  Player
//
//  Created by 常仲伟 on 2022/4/21.
//

#include "AVCodecHandler.hpp"
#include "QueueDef.h"
#include "YUVDataDefine.h"

#if !defined(MIN)
#define MIN(A,B)  ((A) < (B) ? (A) : (B))
#endif

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
  if (avformat_open_input(&m_pFormatCtx, filPath, NULL, NULL) != 0) {
    printf("avformat open input failed...\n");
    return -1;
  }
  
  if (avformat_find_stream_info(m_pFormatCtx, NULL) < 0) {
    printf("avformat find stream failed...\n");
    return -1;
  }
  
  av_dump_format(m_pFormatCtx, 0, filPath, 0);
  printf("---------------------------\n");
  
  m_videoStreamIdx = -1;
  m_audioStreamIdx = -1;
  for (int i=0; i<(int)m_pFormatCtx->nb_streams; i++){
    AVCodecParameters *codecParamters = m_pFormatCtx->streams[i]->codecpar;
    if (codecParamters->codec_type == AVMEDIA_TYPE_VIDEO) {
      m_videoStreamIdx = i;
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
      m_audioStreamIdx = i;
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
  AVStream *videoStream = m_pFormatCtx->streams[m_videoStreamIdx];
  AVStream *audioStream = m_pFormatCtx->streams[m_audioStreamIdx];
  m_vStreamTimeRational = videoStream->time_base;
  m_aStreamTimeRational = audioStream->time_base;
  printf("V Time base num: %d, den: %d\n", m_vStreamTimeRational.num, m_vStreamTimeRational.den);
  printf("A Time base num: %d, den: %d\n", m_aStreamTimeRational.num, m_aStreamTimeRational.den);
  
  printf("Stream IDX: Video: %d, Audio: %d\n", m_videoStreamIdx, m_audioStreamIdx);
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
  std::thread readThread(&AVCodecHandler::doReadMediaFrameThread, this);
  readThread.detach();
  
  std::thread audioThread(&AVCodecHandler::doAudioDecodePlayThread, this);
  audioThread.detach();
  
  std::thread videoThread(&AVCodecHandler::doVideoDecodePlayThread, this);
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
  int retValue = av_read_frame(m_pFormatCtx, packet);
  if (retValue == 0) {
    if (packet->stream_index == m_videoStreamIdx) {
      if (!av_dup_packet(packet)) {// 这个是啥？
        m_videoPacketQueue.enqueue(packet);
      }else {
        freePacket(packet);
      }
    } else if (packet->stream_index == m_audioStreamIdx) {
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

//MARK: -
void AVCodecHandler::doAudioDecodePlayThread(){
  if (m_pFormatCtx == NULL) {
    return;
  }
  if (m_pAudioFrame == NULL) {
    m_pAudioFrame = av_frame_alloc();
  }
  while (m_bThreadRunning) {
    m_bThreadRunning = true;
    if(m_eMediaPlayStatus == MediaPlayStatusPause) {
      stdThreadSleep(10);
      continue;
    }
    if (m_audioPacketQueue.isEmpty()){
      stdThreadSleep(10);
      continue;
    }
    
    AVPacket *pkt = (AVPacket *)m_audioPacketQueue.dequeue();
    if (pkt == NULL) {
      break;
    }
    if (!m_bThreadRunning) {
      freePacket(pkt);
      break;
    }
    tickAudioFrameTimerDelay(pkt->pts);
    
    int retValue = avcodec_send_packet(m_pAudioCodecCtx, pkt);
    if (retValue != 0) {
      freePacket(pkt);
      continue;
    }
    int decodeRet = avcodec_receive_frame(m_pAudioCodecCtx, m_pAudioFrame);
    if ( decodeRet == 0 ) {
      // .... 播放音乐
    }
    freePacket(pkt);
  }
  
  m_bThreadRunning = false;
  printf("audio decode shw thread exit...\n");
}

//MARK: -
void AVCodecHandler::doVideoDecodePlayThread(){
  if (m_pFormatCtx == NULL) {
    return;
  }
  if (m_pVideoFrame == NULL) {
    m_pVideoFrame = av_frame_alloc();
  }
  while (m_bThreadRunning) {
    m_bThreadRunning = true;
    if(m_eMediaPlayStatus == MediaPlayStatusPause) {
      stdThreadSleep(10);
      continue;
    }
    if (m_videoPacketQueue.isEmpty()){
      stdThreadSleep(10);
      continue;
    }
    
    AVPacket *pkt = (AVPacket*)m_videoPacketQueue.dequeue();
    if (pkt == NULL) {
      break;
    }
    if (!m_bThreadRunning) {
      freePacket(pkt);
      break;
    }
    tickVideoFrameTimerDelay(pkt->pts);
    
    int retValue = avcodec_send_packet(m_pVideoCodecCtx, pkt);
    if (retValue != 0) {
      freePacket(pkt);
      continue;
    }
    
    int decodeRet = avcodec_receive_frame(m_pVideoCodecCtx, m_pVideoFrame);
    if (decodeRet == 0) {
      // 解码成功渲染
      
    }
    
    freePacket(pkt);
  }
  
  m_bThreadRunning = false;
  printf("video decode shw thread exit...\n");
}

void AVCodecHandler::convertAndRenderVideo(AVFrame* videoFrame,long long ppts){
  if(videoFrame == NULL){
    return;
  }
  
  if(m_pVideoSwsCtx == NULL) {
    //获取sws上下文, MP4,H.264 不需要，因为解码后就是 YUV420
    m_pVideoSwsCtx = sws_getContext(m_pVideoCodecCtx->width,m_pVideoCodecCtx->height,
                                    m_pVideoCodecCtx->pix_fmt,
                                    m_pVideoCodecCtx->width,m_pVideoCodecCtx->height,
                                    AV_PIX_FMT_YUV420P,
                                    SWS_BICUBIC,NULL,NULL,NULL);
  }
  // 视频转码，很耗 cpu
  sws_scale(m_pVideoSwsCtx, (const uint8_t* const*)videoFrame->data,
            videoFrame->linesize, 0,
            m_pVideoCodecCtx->height,
            m_pYUVFrame->data,
            m_pYUVFrame->linesize);
  
  // Y:分量等于h x w, U h/2 x w/2 V h/2 x w/2, | 32--字节对齐
  unsigned int lumaLength   = m_pVideoCodecCtx->height * (MIN(videoFrame->linesize[0], m_pVideoCodecCtx->width));
  unsigned int chromBLength = ((m_pVideoCodecCtx->height)/2) * (MIN(videoFrame->linesize[1], (m_pVideoCodecCtx->width)/2));
  unsigned int chromRLength = ((m_pVideoCodecCtx->height)/2) * (MIN(videoFrame->linesize[2], (m_pVideoCodecCtx->width)/2));
  
  YUVData_Frame *updateYUVFrame = new YUVData_Frame();
  
  updateYUVFrame->luma.length = lumaLength;
  updateYUVFrame->chromaB.length = chromBLength;
  updateYUVFrame->chromaR.length =chromRLength;
  
  updateYUVFrame->luma.dataBuffer=(unsigned char*)malloc(lumaLength);
  updateYUVFrame->chromaB.dataBuffer=(unsigned char*)malloc(chromBLength);
  updateYUVFrame->chromaR.dataBuffer=(unsigned char*)malloc(chromRLength);
  
  copyDecodedFrame420(m_pYUVFrame->data[0],updateYUVFrame->luma.dataBuffer,m_pYUVFrame->linesize[0],
                      m_pVideoCodecCtx->width,m_pVideoCodecCtx->height);
  copyDecodedFrame420(m_pYUVFrame->data[1], updateYUVFrame->chromaB.dataBuffer,m_pYUVFrame->linesize[1],
                      m_pVideoCodecCtx->width / 2,m_pVideoCodecCtx->height / 2);
  copyDecodedFrame420(m_pYUVFrame->data[2], updateYUVFrame->chromaR.dataBuffer,m_pYUVFrame->linesize[2],
                      m_pVideoCodecCtx->width / 2,m_pVideoCodecCtx->height / 2);
  
  updateYUVFrame->width=m_pVideoCodecCtx->width;
  updateYUVFrame->height=m_pVideoCodecCtx->height;
  
  updateYUVFrame->pts = ppts;
  
  if(m_updateVideoCallback) {
    m_updateVideoCallback(updateYUVFrame, m_userDataVideo);
  }
  
  if(updateYUVFrame->luma.dataBuffer){
    free(updateYUVFrame->luma.dataBuffer);
    updateYUVFrame->luma.dataBuffer=NULL;
  }
  
  if(updateYUVFrame->chromaB.dataBuffer){
    free(updateYUVFrame->chromaB.dataBuffer);
    updateYUVFrame->chromaB.dataBuffer=NULL;
  }
  
  if(updateYUVFrame->chromaR.dataBuffer){
    free(updateYUVFrame->chromaR.dataBuffer);
    updateYUVFrame->chromaR.dataBuffer=NULL;
  }
  
  if(updateYUVFrame){
    delete updateYUVFrame;
    updateYUVFrame = NULL;
  }
}
void AVCodecHandler::convertAndPlayAudio(AVFrame* decodedFrame){
  
}
void AVCodecHandler::copyDecodedFrame(uint8_t* src, uint8_t* dist,int linesize, int width, int height){
  
}
void AVCodecHandler::copyDecodedFrame420(uint8_t* src, uint8_t* dist,int linesize, int width, int height){
  width = MIN(linesize, width);
  for (int i=0; i<height; ++i) {
    memcpy(dist, src, width);
    dist += width;
    src += linesize;
  }
}

float AVCodecHandler::getAudioTimestampFramPTS(int64_t pts){
  float aTimeStamp = pts * av_q2d(m_aStreamTimeRational);
  return aTimeStamp;
}

float AVCodecHandler::getVideoTimestampFramPTS(int64_t pts){
  float vTimeStamp = pts * av_q2d(m_vStreamTimeRational);
  return vTimeStamp;
}

void AVCodecHandler::tickVideoFrameTimerDelay(int64_t pts){
  if (m_vStreamTimeRational.den <= 0) {
    return;
  }
  float currentVideoTimestamp = getVideoTimestampFramPTS(pts);
  float diffTime = (currentVideoTimestamp - m_nLastAudioTimestamp) * 1000;
  int sleepTime = (int)diffTime;
  if (sleepTime > 0 && sleepTime < 5000) {
    stdThreadSleep(sleepTime);
  }
}

void AVCodecHandler::tickAudioFrameTimerDelay(int64_t pts){
  if (m_aStreamTimeRational.den <= 0) {
    return;
  }
  m_nCurrentAudioTimestamp = getAudioTimestampFramPTS(pts);
  int diffTime = (int)(m_nCurrentAudioTimestamp - m_nLastAudioTimestamp);
  if (abs(diffTime) >= 1) {
    // 更新pts 到GUI 界面上
    
  }
  m_nLastAudioTimestamp = m_nCurrentAudioTimestamp;
  return;
}

void AVCodecHandler::freePacket(AVPacket* pkt) {
  if(pkt == NULL ){
    return;
  }
  av_free_packet(pkt);
  free(pkt);
}

void AVCodecHandler::resetAllMediaPlayerParameters() {
  m_pFormatCtx       = NULL;
  m_pVideoCodecCtx   = NULL;
  m_pAudioCodecCtx   = NULL;
//  m_pYUVFrame        = NULL;
  m_pVideoFrame      = NULL;
  m_pAudioFrame      = NULL;
//  m_pAudioSwrCtx     = NULL;
//  m_pVideoSwsCtx     = NULL;
//  m_pYUV420Buffer    = NULL;
//  m_pSwrBuffer       = NULL;
  
  m_videoWidth   = 0;
  m_videoHeight  = 0;
  
  m_videoPathString = "";
  
  m_videoStreamIdx = -1;
  m_audioStreamIdx = -1;
  
  m_bReadFileEOF   = false;
  
//  m_nSeekingPos      = 0;
  
  
//  m_nCurrAudioTimeStamp = 0.0f;
//  m_nLastAudioTimeStamp = 0.0f;
  
//  m_sampleRate = 44100;
//  m_sampleSize = 16;
//  m_channel    = 2;
  
//  m_volumeRatio = 1.0f;
//  m_swrBuffSize = 0;
  
  m_vStreamTimeRational = av_make_q(0,0);
  m_aStreamTimeRational = av_make_q(0,0);
  
//  m_mediaType = MEDIATYPE_VIDEO;
  m_eMediaPlayStatus = MediaPlayStatusStop;
}
