#import <Foundation/Foundation.h>
#include "MediaController.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        MediaController* mediaController = [[MediaController alloc] init];
        NSString* input = nil;
        do {
            char inputString[100];
            printf("Enter command: ");
            scanf("%s", inputString);
            input = [NSString stringWithUTF8String:inputString];
            if ([input caseInsensitiveCompare:@"1"] == NSComparisonResult::NSOrderedSame) {
                [mediaController getUserMedia];
            } else if ([input caseInsensitiveCompare:@"2"] == NSComparisonResult::NSOrderedSame) {
                [mediaController createReceiveForwardingVoiceChannel];
            } else if ([input caseInsensitiveCompare:@"3"] == NSComparisonResult::NSOrderedSame) {
                [mediaController createSendForwardingVoiceChannel];
            }
        } while ([input caseInsensitiveCompare:@"exit"] != NSComparisonResult::NSOrderedSame);
    }
    return 0;
}
