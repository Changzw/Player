//
//  ViewController.m
//  Player
//
//  Created by 常仲伟 on 2022/4/21.
//

#import "RootViewController.h"

@interface RootViewController ()

@end

@implementation RootViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  
  [self.view setBackgroundColor:[UIColor blueColor]];
  avcodec_register_all();
  const char *config = avcodec_configuration();
  
  printf("%d\n, %s\n",  avcodec_version(), config);
}


@end
