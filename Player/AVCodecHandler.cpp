//
//  AVCodecHandler.cpp
//  Player
//
//  Created by 常仲伟 on 2022/4/21.
//

#include "AVCodecHandler.hpp"

AVCodecHandler::AVCodecHandler() {
    avcodec_register_all();
    const char *config = avcodec_configuration();
  
    printf("%d\n, %s\n",  avcodec_version(), config);
}

AVCodecHandler::~AVCodecHandler() {
  
}

void AVCodecHandler::setVideoFilePath(const std::string &path){
  
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
  
  return 0;
}
int AVCodecHandler::destoryVideoCodec(){
  
  return 0;
}
void AVCodecHandler::startPlayVideo() {
  
}
void AVCodecHandler::stopPlayVideo(){
  
}
