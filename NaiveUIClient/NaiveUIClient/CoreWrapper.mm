//
//  CoreWrapper.m
//  NaiveUIClient
//
//  Created by tqtifnypmb on 09/01/2018.
//  Copyright © 2018 tqtifnypmb. All rights reserved.
//

#import "CoreWrapper.h"
#include "Core.h"

using namespace brick;
@interface CoreWrapper() {
    std::unique_ptr<Core> _core;
}

@end

@implementation CoreWrapper

+ (instancetype)sharedInstance {
    static dispatch_once_t onceToken;
    static CoreWrapper* instance;
    dispatch_once(&onceToken, ^{
        instance = [CoreWrapper new];
    });
    return instance;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        const char* ip = "127.0.0.1";
        int port = 9999;
        
        _core = std::make_unique<Core>(ip, port);
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
            _core->run();
        });
    }
    sleep(2);
    return self;
}
@end
