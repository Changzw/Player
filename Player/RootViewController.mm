//
//  ViewController.m
//  Player
//
//  Created by 常仲伟 on 2022/4/21.
//

#import "RootViewController.h"
#import "AVCodecHandler.hpp"

@interface RootViewController () {
  AVCodecHandler _playerHandler;
}
@end

@implementation RootViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  
  [self.view setBackgroundColor:[UIColor blueColor]];
  NSString *filePath = [NSBundle.mainBundle pathForResource:@"out10s.mp4" ofType:nil];
  _playerHandler.setVideoFilePath(filePath.UTF8String);
  _playerHandler.initVideoCodec();
} 
@end
