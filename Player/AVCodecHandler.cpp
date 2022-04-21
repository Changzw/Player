//
//  AVCodecHandler.cpp
//  Player
//
//  Created by 常仲伟 on 2022/4/21.
//

#include "AVCodecHandler.hpp"

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
  
  printf("Stream IDX: Video: %d, Audio: %d", m_videoStreamIndex, m_audioStreamIndex);
  return 0;
}

int AVCodecHandler::destoryVideoCodec(){
  
  return 0;
}

void AVCodecHandler::startPlayVideo() {
  
}

void AVCodecHandler::stopPlayVideo(){
  
}
