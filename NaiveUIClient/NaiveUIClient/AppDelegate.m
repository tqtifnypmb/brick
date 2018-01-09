//
//  AppDelegate.m
//  NaiveUIClient
//
//  Created by tqtifnypmb on 08/01/2018.
//  Copyright © 2018 tqtifnypmb. All rights reserved.
//

#import "AppDelegate.h"
#import "ViewController.h"

@interface AppDelegate ()
@property (strong, nonatomic) NSMutableArray<NSWindow*>* windows;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
}

- (void)applicationWillBecomeActive:(NSNotification *)notification {
    
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (IBAction)newView:(id)sender {
    if (self.windows == nil) {
        self.windows = [[NSMutableArray alloc] init];
    }
    
    NSStoryboard* storyboard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
    ViewController* vc = [storyboard instantiateControllerWithIdentifier:@"ViewController"];
    NSWindow* w = [NSWindow windowWithContentViewController:vc];
    [w makeKeyAndOrderFront:nil];
    [self.windows addObject:w];
}

@end
