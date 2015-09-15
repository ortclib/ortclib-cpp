/*
 
 Copyright (c) 2015, Hookflash Inc.
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
 */

#pragma once

#ifndef ORTC_TEST_CONFIG_H_598d68cd208b42966da2df998901c964
#define ORTC_TEST_CONFIG_H_598d68cd208b42966da2df998901c964

#define ORTC_TEST_FIFO_LOGGING_FILE "/tmp/openpeer.fifo"

#define ORTC_TEST_USE_STDOUT_LOGGING     (false)
#define ORTC_TEST_USE_FIFO_LOGGING       (false)
#define ORTC_TEST_USE_TELNET_LOGGING     (true)
#define ORTC_TEST_USE_DEBUGGER_LOGGING   (false)

#define ORTC_TEST_TELNET_LOGGING_PORT         (59999)
#define ORTC_TEST_TELNET_SERVER_LOGGING_PORT  (51999)

#define ORTC_TEST_DO_ICE_GATHERER_TEST                    (false)
#define ORTC_TEST_DO_ICE_TRANSPORT_TEST                   (false)
#define ORTC_TEST_DO_DTLS_TRANSPORT_TEST                  (false)
#define ORTC_TEST_DO_SRTP_TEST                            (false)
#define ORTC_TEST_DO_SCTP_TRANSPORT_TEST                  (false)
#define ORTC_TEST_DO_RTP_PACKET_TEST                      (true)


#define ORTC_TEST_STUN_SERVER             "stun.vline.com"

#define ORTC_TEST_REFLEXIVE_UDP_IPS       1

#define ORTC_TEST_RELAY_UDP_IPS           1

#define OPENPEER_SERVICE_TEST_TURN_USERNAME        "robin@hookflash.com"
#define OPENPEER_SERVICE_TEST_TURN_PASSWORD        "DxuitmVR3cNX"

#define OPENPEER_SERVICE_TEST_TURN_SERVER_DOMAIN_VIA_A_RECORD_1   "numb.viagenie.ca"


//#define ORTC_TEST_DNS_ZONE "dnstest.hookflash.me"
//
//// true = running as a client, false = running as a server
//#define ORTC_TEST_RUNNING_AS_CLIENT                        (true)
//#define ORTC_TEST_RUDP_SERVER_IP                           "192.168.2.220"
//#define ORTC_TEST_RUDP_SERVER_PORT                         50000
//
//#define ORTC_TEST_DNS_PROVIDER_RESOLVES_BOGUS_DNS_A_RECORDS    (false)
//#define ORTC_TEST_DNS_PROVIDER_RESOLVES_BOGUS_DNS_AAAA_RECORDS (false)
//
//#define ORTC_TEST_TURN_SERVER_DOMAIN   "dnstest.hookflash.me"
//#define ORTC_TEST_TURN_USERNAME        "toto"
//#define ORTC_TEST_TURN_PASSWORD        "toto"
//
//#define ORTC_TEST_TURN_SERVER_DOMAIN_VIA_A_RECORD_1   "turn1.dnstest.hookflash.me"
//#define ORTC_TEST_TURN_SERVER_DOMAIN_VIA_A_RECORD_2   "turn2.dnstest.hookflash.me"
//
//#define ORTC_TEST_STUN_SERVER         "stun.vline.com"
//// This should be set to value based on http://www.whatismyip.com/ to get your current IP address
//#define ORTC_TEST_WHAT_IS_MY_IP        "174.91.34.228"
//
//

#endif //ORTC_TEST_CONFIG_H_598d68cd208b42966da2df998901c964
