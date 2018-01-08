//
//  ViewController.m
//  NaiveUIClient
//
//  Created by tqtifnypmb on 08/01/2018.
//  Copyright © 2018 tqtifnypmb. All rights reserved.
//

#import "ViewController.h"

#include "Client.h"
#include "Core.h"
#include "Request.h"

using namespace brick;

@interface ViewController() <NSTextViewDelegate> {
    std::unique_ptr<Client> _client;
    std::unique_ptr<Core> _core;
}

@property (weak) IBOutlet NSTextView *textView;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    _core = std::make_unique<Core>("127.0.0.1", 9999);
    
    self.textView.delegate = self;
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
        NSLog(@"Core");
        _core->run();
    });
    
    sleep(2);
    _client = std::make_unique<Client>("127.0.0.1", 9999, nullptr, nullptr);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
        _client->loop();
    });
}

void serverCallback(Client& client) {
    auto respJson = client.read();
    auto resp = Request::fromJson(respJson);
    if (resp.method() == Request::MethodType::update) {
        
    }
}

- (void)textViewDidChangeSelection:(NSNotification *)notification {
    
}

- (void)textDidChange:(NSNotification *)notification {
    
}

@end
