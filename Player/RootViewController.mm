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
  
}


@end
