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
