//
//  ViewController.m
//  NaiveUIClient
//
//  Created by tqtifnypmb on 08/01/2018.
//  Copyright © 2018 tqtifnypmb. All rights reserved.
//

#import "ViewController.h"
#import "CoreWrapper.h"

#include "Request.h"

#include <arpa/inet.h>
#include <vector>

using namespace brick;

@interface ViewController() <NSTextViewDelegate> {
    CFSocketRef _client;
    CFRunLoopSourceRef _clientSource;
    int _viewId;
    size_t _reqId;
}

@property (unsafe_unretained) IBOutlet NSTextView *textView;
@property (assign, nonatomic) BOOL isFirstTime;
@property (assign, nonatomic) BOOL editingText;
@property (assign, nonatomic) BOOL blockRpc;
@property (strong, nonatomic) NSMutableArray<NSData*>* pendingReqs;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
   
    self.pendingReqs = [[NSMutableArray alloc] init];
    self.textView.delegate = self;
    _viewId = -1;
    _reqId = 0;
    self.isFirstTime = YES;
    
    self.filePath = @"abc.txt";
    
    [CoreWrapper sharedInstance];
    [self _createClientSocket];
}

- (IBAction)blockRpcClicked:(NSButton*)sender {
    if (self.blockRpc) {
        [sender setTitle:@"Block Rpc"];
    } else {
        [sender setTitle:@"Unblock Rpc"];
    }
    self.blockRpc = !self.blockRpc;
    
    if (!self.blockRpc) {
        for (NSData* toSend in self.pendingReqs) {
            CFSocketError error = CFSocketSendData(_client, NULL, (__bridge_retained CFDataRef)toSend, 0);
            if (error != kCFSocketSuccess) {
                std::terminate();
            }
        }
        [self.pendingReqs removeAllObjects];
    }
}

#pragma mark - Setup

- (void)viewWillAppear {
    if (self.isFirstTime) {
        self.isFirstTime = NO;
        [self newView];
    }
}

static void coreCallback(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info) {
    CFDataRef respData = (CFDataRef)data;
    NSData* resp = (__bridge NSData*)respData;
    NSString* respStr = [[NSString alloc] initWithData:resp encoding:NSUTF8StringEncoding];
    
    CFSocketContext ctx;
    CFSocketGetContext(s, &ctx);
    ViewController* vc = (__bridge ViewController*)ctx.info;
    
    for (NSString* s in [respStr componentsSeparatedByString:@"\n"]) {
        if (s.length == 0) continue;
        
        std::string cStr = [vc NSStringToString:s];
        auto req = Request::fromJson(cStr);
        
        if (req.method() == Request::MethodType::response) {
            [vc parseResponse:req];
        } else {
            [vc handleUpdate:req];
        }
    }
}

- (void)_createClientSocket {
    CFSocketContext ctx;
    ctx.info = (__bridge void*)self;
    ctx.retain = nullptr;
    ctx.release = nullptr;
    ctx.copyDescription = nullptr;
    _client = CFSocketCreate(kCFAllocatorDefault, PF_INET, SOCK_STREAM, 0, kCFSocketDataCallBack, coreCallback, &ctx);
    
    const char* ip = "127.0.0.1";
    int port = 9999;
    sockaddr_in sin;
    inet_pton(AF_INET, ip, &sin.sin_addr.s_addr);
    sin.sin_port = htons(port);
    sin.sin_family = AF_INET;
    
    NSData* address = [NSData dataWithBytes:(const struct sockaddr*)&sin length:sizeof(sin)];
    CFSocketError error = CFSocketConnectToAddress(_client, (__bridge CFDataRef)address, 0);
    if (error != kCFSocketSuccess) {
        std::terminate();
    }
    
    _clientSource = CFSocketCreateRunLoopSource(kCFAllocatorDefault, _client, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), _clientSource, kCFRunLoopDefaultMode);
}

#pragma mark - Parse Core Response

- (void)parseResponse:(Request)resp {
    NSLog(@"%s", resp.toJson().c_str());
    
    if (_viewId == -1) {
        _viewId = resp.getParams<int>("viewId");
        [self scroll:NSMakeRange(0, 100)];
    }
}

- (void)handleUpdate:(Request)req {
    NSLog(@"update %s", req.toJson().c_str());
    
    NSMutableString* str = [NSMutableString stringWithString:self.textView.textStorage.string];
    for (const auto& update : req.params()) {
        const auto& op = update["op"].get<std::string>();
        auto begIndex = update["range"][0].get<int>();
        auto endIndex = update["range"][1].get<int>();
        auto begRow = update["begRow"].get<int>();
        auto endRow = update["endRow"].get<int>();
        if (op == "insert") {
            auto bytes = update["text"].get<std::string>();
            [str insertString:[NSString stringWithCString:bytes.c_str() encoding:NSUTF8StringEncoding] atIndex:begIndex];
        } else if (op == "erase") {
            [str deleteCharactersInRange:NSMakeRange(begIndex, endIndex)];
        }
    }
    self.textView.string = str;
}

#pragma mark - Client Request

- (void)newView {
    if (self.filePath.length == 0) {
        [self sendRequest:Request::MethodType::new_view params:nil];
    } else {
        [self sendRequest:Request::MethodType::new_view params:@{@"filePath": self.filePath}];
    }
}

- (void)select:(NSRange)sel {
    [self sendRequest:Request::MethodType::select params:@{@"viewId": @(_viewId), @"range": @[@(sel.location), @(sel.length)]}];
}

- (void)insertText:(NSString*)str range:(NSRange)range; {
    [self select:range];
    [self sendRequest:Request::MethodType::insert params:@{@"viewId": @(_viewId), @"bytes": str}];
}

- (void)erase:(NSRange)range {
    [self select:range];
    [self sendRequest:Request::MethodType::erase params:@{@"viewId": @(_viewId)}];
}

- (void)scroll:(NSRange)range {
    [self sendRequest:Request::MethodType::scroll params:@{@"viewId": @(_viewId), @"range": @[@(range.location), @(range.length)]}];
}

- (void)region {
    [self sendRequest:Request::MethodType::text params:@{@"viewId": @(_viewId)}];
}

- (void)sendRequest:(Request::MethodType)reqType params:(NSDictionary*)params {
    NSString* reqStr;
    if (params == nil) {
        Request req = Request(_reqId++, reqType);
        auto json = req.toJson();
        reqStr = [[NSString alloc] initWithBytes:json.c_str() length:json.length() encoding:NSUTF8StringEncoding];
    } else {
        NSData* jsonData = [NSJSONSerialization dataWithJSONObject:params options:0 error:nil];
        NSString* jsonStr = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        std::string cJson = [self NSStringToString:jsonStr];
        auto pJson = nlohmann::json::parse(cJson);
        Request req = Request(_reqId++, reqType, pJson);
        auto json = req.toJson();
        reqStr = [[NSString alloc] initWithBytes:json.c_str() length:json.length() encoding:NSUTF8StringEncoding];
    }
    
    NSData* toSend = [NSData dataWithBytes:reqStr.UTF8String length:[reqStr lengthOfBytesUsingEncoding:NSUTF8StringEncoding]];
    if (self.blockRpc) {
        [self.pendingReqs addObject:toSend];
    } else {
        CFSocketError error = CFSocketSendData(_client, NULL, (__bridge_retained CFDataRef)toSend, 0);
        if (error != kCFSocketSuccess) {
            std::terminate();
        }
    }
}

- (std::string)NSStringToString:(NSString*)str {
    const char* cStr = [str cStringUsingEncoding:NSUTF8StringEncoding];
    return std::string(cStr, [str lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
}

#pragma mark - TextView Delegate

- (void)textViewDidChangeSelection:(NSNotification *)notification {
    if (self.editingText) {
        self.editingText = NO;
        return;
    }
    
    //NSRange sel = self.textView.selectedRange;
    //[self select:sel];
}

- (BOOL)textView:(NSTextView *)textView shouldChangeTextInRange:(NSRange)affectedCharRange replacementString:(NSString *)replacementString {
    self.editingText = YES;
    
    if (replacementString.length == 0) {
        if (textView.string.length == 0) {
            return YES;
        } else {
            [self erase:affectedCharRange];
        }
    } else {
        [self insertText:replacementString range:affectedCharRange];
    }
    return YES;
}

- (void)textDidChange:(NSNotification *)notification {
    self.editingText = NO;
    [self region];
}

@end
