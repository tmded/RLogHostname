#ifndef _REMOTE_LOG_H_
#define _REMOTE_LOG_H_

#import <netinet/in.h>
#import <sys/socket.h>
#import <unistd.h>
#import <arpa/inet.h>
#import <netdb.h>
#import <sys/socket.h>
#import <netinet/in.h>
#import <SystemConfiguration/SystemConfiguration.h>

// change this to match your destination (server) IP address
#define RLOG_IP_ADDRESS "tomslaptop.local"
#define RLOG_PORT 11909


/*
Connectivity testing code pulled from Apple's Reachability Example: https://developer.apple.com/library/content/samplecode/Reachability
 */
static bool hasConnectivity (){
    struct sockaddr_in zeroAddress;
    bzero(&zeroAddress, sizeof(zeroAddress));
    zeroAddress.sin_len = sizeof(zeroAddress);
    zeroAddress.sin_family = AF_INET;

    SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, (const struct sockaddr*)&zeroAddress);
    if (reachability != NULL) {
        //NetworkStatus retVal = NotReachable;
        SCNetworkReachabilityFlags flags;
        if (SCNetworkReachabilityGetFlags(reachability, &flags)) {
            if ((flags & kSCNetworkReachabilityFlagsReachable) == 0)
            {
                // If target host is not reachable
                return NO;
            }

            if ((flags & kSCNetworkReachabilityFlagsConnectionRequired) == 0)
            {
                // If target host is reachable and no connection is required
                //  then we'll assume (for now) that your on Wi-Fi
                return YES;
            }


            if ((((flags & kSCNetworkReachabilityFlagsConnectionOnDemand ) != 0) ||
                 (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic) != 0))
            {
                // ... and the connection is on-demand (or on-traffic) if the
                //     calling application is using the CFSocketStream or higher APIs.

                if ((flags & kSCNetworkReachabilityFlagsInterventionRequired) == 0)
                {
                    // ... and no [user] intervention is needed
                    return YES;
                }
            }

            if ((flags & kSCNetworkReachabilityFlagsIsWWAN) == kSCNetworkReachabilityFlagsIsWWAN)
            {
                // ... but WWAN connections are OK if the calling application
                //     is using the CFNetwork (CFSocketStream?) APIs.
                return YES;
            }
        }
    }

    return NO;
}


__attribute__((unused)) static void RLogv(NSString* format, va_list args)
{
    #if DEBUG
        NSString* str = [[NSString alloc] initWithFormat:format arguments:args];

        int sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sd <= 0)
        {
            NSLog(@"[RemoteLog] Error: Could not open socket");
            return;
        }

        int broadcastEnable = 1;
        int ret = setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
        if (ret)
        {
            NSLog(@"[RemoteLog] Error: Could not open set socket to broadcast mode");
            close(sd);
            return;
        }
        struct sockaddr_in broadcastAddr;                                                                                        
        memset(&broadcastAddr, 0, sizeof broadcastAddr);                                                                
        broadcastAddr.sin_family = AF_INET;

        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = PF_UNSPEC;        // PF_INET if you want only IPv4 addresses
        hints.ai_protocol = IPPROTO_TCP;
        if (!(hasConnectivity()))return;
        
        struct addrinfo *addrs, *addr;
        char *ipbuf;
        ipbuf = strdup(" ");;
        getaddrinfo(RLOG_IP_ADDRESS, NULL, &hints, &addrs);
        for (addr = addrs; addr; addr = addr->ai_next) {
            char host[NI_MAXHOST];
            getnameinfo(addr->ai_addr, addr->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
            ipbuf = host;
            NSLog(@"bruh");
        }
        freeaddrinfo(addrs);


        inet_pton(AF_INET, ipbuf, &broadcastAddr.sin_addr);
        broadcastAddr.sin_port = htons(RLOG_PORT);
        char* request = (char*)[str UTF8String];
        ret = sendto(sd, request, strlen(request), 0, (struct sockaddr*)&broadcastAddr, sizeof broadcastAddr);
        if (ret < 0)
        {
            NSLog(@"[RemoteLog] Error: Could not send broadcast");
            close(sd);
            return;
        }
        close(sd);
    #endif
}

__attribute__((unused)) static void RLog(NSString* format, ...)
{
    #if DEBUG
        va_list args;
        va_start(args, format);
        RLogv(format, args);
        va_end(args);
    #endif
}
#endif
