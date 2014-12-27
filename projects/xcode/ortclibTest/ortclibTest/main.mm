#import <Foundation/Foundation.h>
#include "MediaController.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        MediaController* mediaController = [[MediaController alloc] init];
        NSString* input = nil;
        do {
            char inputString[100];
            inputString[0] = 0;
            printf("Enter command:\n");
            printf("1 - Create Receive Audio Track\n");
            printf("2 - Create Send Audio Track\n");
            printf("3 - Delete Send Audio Track\n");
            printf("4 - Delete Receive Audio Track\n");
            scanf("%s", inputString);
            input = [NSString stringWithUTF8String:inputString];
            if ([input caseInsensitiveCompare:@"1"] == NSComparisonResult::NSOrderedSame) {
                printf("Create Receive Audio Track\n");
                printf("Enter port number: ");
                scanf("%s", inputString);
                NSString* inputPortNumber = [NSString stringWithUTF8String:inputString];
                NSInteger portNumber = [inputPortNumber integerValue];
                [mediaController createReceiveForwardingAudioTrackWithPortNumber:portNumber];
            } else if ([input caseInsensitiveCompare:@"2"] == NSComparisonResult::NSOrderedSame) {
                printf("Create Send Audio Track\n");
                printf("Enter receiver IP address: ");
                scanf("%s", inputString);
                NSString* inputReceiverIPAddress = [NSString stringWithUTF8String:inputString];
                printf("Enter receiver port number: ");
                scanf("%s", inputString);
                NSString* inputReceiverPortNumber = [NSString stringWithUTF8String:inputString];
                NSInteger receiverPortNumber = [inputReceiverPortNumber integerValue];
                printf("Enter source audio track port number: ");
                scanf("%s", inputString);
                NSString* inputSourceAudioTrackPortNumber = [NSString stringWithUTF8String:inputString];
                NSInteger sourceAudioTrackPortNumber = [inputSourceAudioTrackPortNumber integerValue];
                [mediaController createSendForwardingAudioTrackWithReceiverIPAddress:inputReceiverIPAddress
                                                               andReceiverPortNumber:receiverPortNumber
                                                       andSourceAudioTrackPortNumber:sourceAudioTrackPortNumber];
            } else if ([input caseInsensitiveCompare:@"3"] == NSComparisonResult::NSOrderedSame) {
                printf("Delete Send Audio Track\n");
            } else if ([input caseInsensitiveCompare:@"4"] == NSComparisonResult::NSOrderedSame) {
                printf("Delete Receive Audio Track\n");
            }
        } while ([input caseInsensitiveCompare:@"exit"] != NSComparisonResult::NSOrderedSame);
    }
    return 0;
}
