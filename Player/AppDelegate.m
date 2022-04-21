//
//  AppDelegate.m
//  Player
//
//  Created by 常仲伟 on 2022/4/21.
//

#import "AppDelegate.h"
#import "RootViewController.h"

@interface AppDelegate ()
@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
  
  self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
  
  RootViewController* rootcontroller = [[RootViewController alloc] init];
  UINavigationController* navController=[[UINavigationController alloc] initWithRootViewController:rootcontroller];
  [navController.navigationItem setTitle:@"Video Player"];
  
  self.window.rootViewController = navController;
  [self.window makeKeyAndVisible];
  return YES;
}
@end
