//
//  AppDelegate.m
//  Player
//
//  Created by 常仲伟 on 2022/4/21.
//

#import "AppDelegate.h"
#import "RootViewController.h"

@interface AppDelegate ()
@property(nonatomic, strong) UIWindow *windown;
@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
  _windown = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
  
  RootViewController* rootcontroller = [[RootViewController alloc] init];
  UINavigationController* navController=[[UINavigationController alloc] initWithRootViewController:rootcontroller];
  [navController.navigationItem setTitle:@"Video Player"];
  
  self.window.rootViewController = navController;
  
  [self.window makeKeyAndVisible];
  
  return YES;
}


#pragma mark - UISceneSession lifecycle


- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options {
  // Called when a new scene session is being created.
  // Use this method to select a configuration to create the new scene with.
  return [[UISceneConfiguration alloc] initWithName:@"Default Configuration" sessionRole:connectingSceneSession.role];
}


- (void)application:(UIApplication *)application didDiscardSceneSessions:(NSSet<UISceneSession *> *)sceneSessions {
  // Called when the user discards a scene session.
  // If any sessions were discarded while the application was not running, this will be called shortly after application:didFinishLaunchingWithOptions.
  // Use this method to release any resources that were specific to the discarded scenes, as they will not return.
}


@end
