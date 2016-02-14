/*

 Copyright (c) 2016, Hookflash Inc.
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

#include <ortc/internal/types.h>


#ifdef USE_ETW
#include "ortc_ETWTracing.h"
#else

// Comment the following line to test inline versions of the same macros to test compilation
#define ORTC_USE_NOOP_EVENT_TRACE_MACROS

// NO-OP VERSIONS OF ALL TRACING MACROS
#ifdef ORTC_USE_NOOP_EVENT_TRACE_MACROS

#define EventWriteOrtcCreate(xStr_Method, xPUID)
#define EventWriteOrtcDestroy(xStr_Method, xPUID)

#define EventWriteOrtcCertificateCreate(xStr_Method, xPUID, xStr_KeyGenAlgorithm, xStr_Name, xStr_NamedCurve, xsize_t_KeyLength, xsize_t_RandomBits, xStr_PublicExponenetLength, xlong_long_LifetimeInSeconds, xlong_long_NotBeforeWindowInSeconds, xStr_Expires)
#define EventWriteOrtcCertificateDestroy(xStr_Method, xPUID)
#define EventWriteOrtcCertificateGenerated(xStr_Method, xPUID, xBool_Success)
#define EventWriteOrtcCertificateFingerprint(xStr_Method, xPUID, xStr_FingerprintAlgorithm, xStr_FingerprintValue)

#define EventWriteOrtcDataChannelCreate(xStr_Method, xPUID, xPUID_DataTransport, xStr_Parameters, xBool_Incoming, xWORD_SessionID)
#define EventWriteOrtcDataChannelDestroy(xStr_Method, xPUID)
#define EventWriteOrtcDataChannelClose(xStr_Method, xPUID)
#define EventWriteOrtcDataChannelCancel(xStr_Method, xPUID)
#define EventWriteOrtcDataChannelStep(xStr_Method, xPUID)
#define EventWriteOrtcDataChannelStateChangedEventFired(xStr_Method, xPUID, xStr_State)
#define EventWriteOrtcDataChannelErrorEventFired(xStr_Method, xPUID, xWORD_Error, xStr_ErrorReason)

#define EventWriteOrtcDataChannelSendControlOpen(xStr_Method, xPUID, xBYTE_MessageType, xBYTE_ChannelType, xWORD_Priority, xDWORD_ReliabilityParameter, xWORD_LabelLength, xWORD_ProtocolLength, xStr_Label, xStr_Protocol)
#define EventWriteOrtcDataChannelSendControlAck(xStr_Method, xPUID, xBYTE_MessageType)

#define EventWriteOrtcDataChannelBufferedAmountLowThresholdChanged(xStr_Method, xPUID, xsize_t_NewThreshold, xsize_t_mOldThreshold, xsize_t_OutgoingBufferFillSize, xBool_BufferedAmountLowThresholdFired)

#define EventWriteOrtcDataChannelSendString(xStr_Method, xPUID, xStr_Data)
#define EventWriteOrtcDataChannelSendBinary(xStr_Method, xPUID, xPtr_Buffer, xsize_t_BufferSizeInBytes)

#define EventWriteOrtcDataChannelOutgoingBufferPacket(xStr_Method, xPUID,  xUInt_Type, xWORD_SessionID, xBool_Ordered, xlong_long_MaxPacketLifetimeInMilliseconds, xBool_HasMaxRetransmits, xDWORD_MaxRetransmits, xPtr_Buffer, xsize_t_BufferSizeInBytes)
#define EventWriteOrtcDataChannelOutgoingBufferPacketDelivered(xStr_Method, xPUID,  xUInt_Type, xWORD_SessionID, xBool_Ordered, xlong_long_MaxPacketLifetimeInMilliseconds, xBool_HasMaxRetransmits, xDWORD_MaxRetransmits, xPtr_Buffer, xsize_t_BufferSizeInBytes)

#define EventWriteOrtcDataChannelReceivedControlOpen(xStr_Method, xPUID, xBool_Incoming, xBYTE_MessageType, xBYTE_ChannelType, xWORD_Priority, xDWORD_ReliabilityParameter, xWORD_LabelLength, xWORD_ProtocolLength, xStr_Label, xStr_Protocol)
#define EventWriteOrtcDataChannelReceivedControlAck(xStr_Method, xPUID, xBYTE_MessageType)

#define EventWriteOrtcDataChannelMessageFiredEvent(xStr_Method, xPUID,  xUInt_Type, xWORD_SessionID, xWORD_SequenceNumber, xDWORD_Timestamp, xInt_Flags, xPtr_Buffer, xsize_t_BufferSizeInBytes)

#define EventWriteOrtcDataChannelSCTPTransportDeliverOutgoingPacket(xStr_Method, xPUID,  xUInt_Type, xWORD_SessionID, xBool_Ordered, xlong_long_MaxPacketLifetimeInMilliseconds, xBool_HasMaxRetransmits, xDWORD_MaxRetransmits, xPtr_Buffer, xsize_t_BufferSizeInBytes)

#define EventWriteOrtcDataChannelSCTPTransportRequestShutdown(xStr_Method, xPUID)
#define EventWriteOrtcDataChannelSCTPTransportRequestNotifyClosed(xStr_Method, xPUID)
#define EventWriteOrtcDataChannelSCTPTransportStateChanged(xStr_Method, xPUID)
#define EventWriteOrtcDataChannelSCTPTransportSendReady(xStr_Method, xPUID)
#define EventWriteOrtcDataChannelSCTPTransportSendReadyFailure(xStr_Method, xPUID, xWORD_Error, xStr_ErrorReason)
#define EventWriteOrtcDataChannelSCTPTransportReceivedIncomingPacket(xStr_Method, xPUID,  xUInt_Type, xWORD_SessionID, xWORD_SequenceNumber, xDWORD_Timestamp, xInt_Flags, xPtr_Buffer, xsize_t_BufferSizeInBytes)

#define EventWriteOrtcDtlsTransportCreate(xStr_Method, xPUID, xPUID_IceTransport, xStr_Component, xsize_t_MaxPendingDTLSBuffer, xsize_t_MaxPendingRTPPackets, xsize_t_TotalCertificates)
#define EventWriteOrtcDtlsTransportDestroy(xStr_Method, xPUID)
#define EventWriteOrtcDtlsTransportInitization(xStr_Method, xPUID, xPUID_IceTransport, xPUID_RtpListener, xsize_t_TotalCiphers, xsize_t_TotalFingerprints)
#define EventWriteOrtcDtlsTransportStep(xStr_Method, xPUID)
#define EventWriteOrtcDtlsTransportCancel(xStr_Method, xPUID)

#define EventWriteOrtcDtlsTransportInitizationInstallCipher(xStr_Method, xPUID, xStr_Cipher)
#define EventWriteOrtcDtlsTransportInitizationInstallFingerprint(xStr_Method, xPUID, xStr_Algorithm, xStr_Value, xBool_IsLocal)

#define EventWriteOrtcDtlsTransportStart(xStr_Method, xPUID, xStr_Role, xsize_t_TotalFingerprints)
#define EventWriteOrtcDtlsTransportStop(xStr_Method, xPUID)

#define EventWriteOrtcDtlsTransportRoleSet(xStr_Method, xPUID, xStr_Role)

#define EventWriteOrtcDtlsTransportReceivedPacket(xStr_Method, xPUID, xUInt_ViaTransport, xBool_IsDtlsPacket, xPtr_Buffer, xsize_t_BufferSizeInBytes)
#define EventWriteOrtcDtlsTransportReceivedStunPacket(xStr_Method, xPUID, xUInt_ViaTransport)

#define EventWriteOrtcDtlsTransportForwardingEncryptedPacketToSrtpTransport(xStr_Method, xPUID, xPUID_SrtpTransportObjectID, xUInt_ViaTransport, xPtr_Buffer, xsize_t_BufferSizeInBytes)
#define EventWriteOrtcDtlsTransportForwardingPacketToDataTransport(xStr_Method, xPUID, xPUID_DataTransportObjectID, xUInt_ViaTransport, xPtr_Buffer, xsize_t_BufferSizeInBytes)
#define EventWriteOrtcDtlsTransportForwardingPacketToRtpListener(xStr_Method, xPUID, xPUID_RtpListenerObjectID, xUInt_ViaTransport,  xUInt_PacketType, xPtr_Buffer, xsize_t_BufferSizeInBytes)

#define EventWriteOrtcDtlsTransportSendRtpPacket(xStr_Method, xPUID, xUInt_SendOverComponent, xUInt_PacketType, xPtr_Buffer, xsize_t_BufferSizeInBytes)
#define EventWriteOrtcDtlsTransportSendDataPacket(xStr_Method, xPUID, xPtr_Buffer, xsize_t_BufferSizeInBytes)
#define EventWriteOrtcDtlsTransportForwardDataPacketToIceTransport(xStr_Method, xPUID, xPUID_IceTransportObjectID, xPtr_Buffer, xsize_t_BufferSizeInBytes)

#define EventWriteOrtcDtlsTransportSendEncryptedRtpPacket(xStr_Method, xPUID, xPUID_IceTransportObjectID, xUInt_SendOverComponent, xUInt_PacketType, xPtr_Buffer, xsize_t_BufferSizeInBytes)

#define EventWriteOrtcDtlsTransportInternalTimerEventFired(xStr_Method, xPUID, xPUID_TimerID)
#define EventWriteOrtcDtlsTransportInternalIceStateChangeEventFired(xStr_Method, xPUID, xPUID_IceTransportObjectID, xStr_State)
#define EventWriteOrtcDtlsTransportInternalSrtpTransportLifetimeRemainingEventFired(xStr_Method, xPUID, xPUID_SrtpTransportObjectID, xULONG_LeastLifetimeRemainingPercentageForAllKeys, xULONG_OverallLifetimeRemainingPercentage)

#define EventWriteOrtcDtlsTransportStateChangedEventFired(xStr_Method, xPUID, xStr_State)
#define EventWriteOrtcDtlsTransportErrorEventFired(xStr_Method, xPUID, xWORD_Error, xStr_ErrorReason)

#define EventWriteOrtcDtlsTransportSrtpKeyingMaterialSetup(xStr_Method, xPUID, xPUID_SecureTransportObjectID, xStr_Direction, xStr_Cipher, xPtr_KeyingMaterialBuffer, xsize_t_KeyingMaterialBufferSizeInBytes)

#define EventWriteOrtcIceGathererCreate(xStr_Method, xPUID, xPUID_GethererRouterObjectID, xStr_UsernameFragment, xStr_Password, xBool_CreateTCPCandidates, xBool_ContinousGathering, xsize_t_TotalInterfacePolicies, xsize_t_TotalIceServers, xlong_long_ReflexiveInactivityTimeInSeconds, xlong_long_RelayInactivityTimeInSeoncds, xlong_long_MaxBufferingTimeInSeconds, xlong_long_RecheckIPsInSeconds, xsize_t_MaxTotalBuffers, xsize_t_MaxTCPBufferingSizePendingConnectionInBytes, xsize_t_MaxTCPBufferingSizeConnectedInBytes, xBool_GatherPassiveTCP) {}
#define EventWriteOrtcIceGathererDestroy(xStr_Method, xPUID) {}
#define EventWriteOrtcIceGathererStep(xStr_Method, xPUID) {}
#define EventWriteOrtcIceGathererCancel(xStr_Method, xPUID) {}
#define EventWriteOrtcIceGathererCreatedAssociateGatherer(xStr_Method, xPUID, xPUID_AssociatedGathererObjectID) {}

#define EventWriteOrtcIceGathererInitializeInstallInterfaceNameMapping(xStr_Method, xPUID, xStr_InterfaceNameRegularExpresssion, xStr_InterfaceType, xULONG_OrderIndex) {}
#define EventWriteOrtcIceGathererInitializeInstallPreference(xStr_Method, xPUID, xStr_PreferenceType, xStr_Preference, xStr_SubPreference, xDWORD_PreferenceValue) {}

#define EventWriteOrtcIceGathererGather(xStr_Method, xPUID, xBool_ContinuousGathering, xsize_t_TotalInterfacePolicies, xsize_t_TotalIceServers) {}

#define EventWriteOrtcIceGathererStateChangedEventFired(xStr_Method, xPUID, xStr_State) {}
#define EventWriteOrtcIceGathererErrorEventFired(xStr_Method, xPUID, xWORD_Error, xStr_Reason) {}

#define EventWriteOrtcIceGathererAddCandidateEventFired(xStr_Method, xPUID, xStr_LocalHash, xStr_NotifyHash, xStr_InterfaceType, xStr_Foundation, xDWORD_Priority, xDWORD_UnfreezePriority, xStr_Protocol, xStr_IP, xWORD_Port, xStr_CandidateType, xStr_TcpType, xStr_RelatedAddress, xWORD_RelatedPort) {}
#define EventWriteOrtcIceGathererRemoveCandidateEventFired(xStr_Method, xPUID, xStr_LocalHash, xStr_NotifyHash, xStr_InterfaceType, xStr_Foundation, xDWORD_Priority, xDWORD_UnfreezePriority, xStr_Protocol, xStr_IP, xWORD_Port, xStr_CandidateType, xStr_TcpType, xStr_RelatedAddress, xWORD_RelatedPort) {}
#define EventWriteOrtcIceGathererFilterCandidateEventFired(xStr_Method, xPUID,  xUInt_FilterPolicy, xStr_BoundIP, xStr_InterfaceType, xStr_Foundation, xDWORD_Priority, xDWORD_UnfreezePriority, xStr_Protocol, xStr_IP, xWORD_Port, xStr_CandidateType, xStr_TcpType, xStr_RelatedAddress, xWORD_RelatedPort) {}

#define EventWriteOrtcIceGathererInstallIceTransport(xStr_Method, xPUID, xPUID_IceTransportObjectID, xStr_RemoteUsernameFragment) {}
#define EventWriteOrtcIceGathererRemoveIceTransport(xStr_Method, xPUID, xPUID_IceTransportObjectID) {}
#define EventWriteOrtcIceGathererInternalIceTransportStateChangedEventFired(xStr_Method, xPUID, xPUID_IceTransportObjectID) {}

#define EventWriteOrtcIceGathererRemovetAllIceTransportRelatedRoutes(xStr_Method, xPUID, xPUID_IceTransportObjectID) {}
#define EventWriteOrtcIceGathererSendIceTransportPacket(xStr_Method, xPUID, xPUID_IceTransportObjectID, xPUID_RouteID, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}
#define EventWriteOrtcIceGathererSendIceTransportPacketFailed(xStr_Method, xPUID, xPUID_IceTransportObjectID, xPUID_RouteID, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}

#define EventWriteOrtcIceGathererSendIceTransportPacketViaUdp(xStr_Method, xPUID, xPUID_IceTransportObjectID, xPUID_RouteID, xPUID_HostPortObjectID, xStr_RemoteIP, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}
#define EventWriteOrtcIceGathererSendIceTransportPacketViaTcp(xStr_Method, xPUID, xPUID_IceTransportObjectID, xPUID_RouteID, xPUID_TcpPortObjectID, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}
#define EventWriteOrtcIceGathererSendIceTransportPacketViaTurn(xStr_Method, xPUID, xPUID_IceTransportObjectID, xPUID_RouteID, xPUID_TurnObjectID, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}

#define EventWriteOrtcIceGathererDeliverIceTransportIncomingPacket(xStr_Method, xPUID, xPUID_IceTransportObjectID, xPUID_RouteID, xPUID_xRouterRouteID, xBool_WasBuffered, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}
#define EventWriteOrtcIceGathererBufferIceTransportIncomingPacket(xStr_Method, xPUID, xPUID_xRouterRouteID, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}

#define EventWriteOrtcIceGathererDeliverIceTransportIncomingStunPacket(xStr_Method, xPUID, xPUID_IceTransportObjectID, xPUID_RouteID, xPUID_xRouterRouteID, xBool_WasBuffered) {}
#define EventWriteOrtcIceGathererBufferIceTransportIncomingStunPacket(xStr_Method, xPUID, xPUID_xRouterRouteID) {}
#define EventWriteOrtcIceGathererErrorIceTransportIncomingStunPacket(xStr_Method, xPUID, xPUID_IceTransportObjectID, xPUID_RouteID, xPUID_xRouterRouteID) {}

#define EventWriteOrtcIceGathererDisposeBufferedIceTransportIncomingPacket(xStr_Method, xPUID, xPUID_xRouterRouteID, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}
#define EventWriteOrtcIceGathererDisposeBufferedIceTransportIncomingStunPacket(xStr_Method, xPUID, xPUID_xRouterRouteID) {}

#define EventWriteOrtcIceGathererTurnSocketReceivedPacket(xStr_Method, xPUID, xPUID_TurnSocketObjectID, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}
#define EventWriteOrtcIceGathererTurnSocketSendPacket(xStr_Method, xPUID, xPUID_TurnSocketObjectID, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}

#define EventWriteOrtcIceGathererInstallRoute(xStr_Method, xPUID, xPUID_RouteID, xStr_InterfaceType, xStr_Foundation, xDWORD_Priority, xDWORD_UnfreezePriority, xStr_Protocol, xStr_IP, xWORD_Port, xStr_CandidateType, xStr_TcpCandidateType, xStr_RelatedAddress, xWORD_RelatedPort, xStr_RemoteIP, xStr_LastUsed, xPUID_TransportObjectID, xPUID_HostPortObjectID, xPUID_RelayPortObjectID, xPUID_TCPPortObjectID) {}
#define EventWriteOrtcIceGathererRemoveRoute(xStr_Method, xPUID, xPUID_RouteID, xStr_InterfaceType, xStr_Foundation, xDWORD_Priority, xDWORD_UnfreezePriority, xStr_Protocol, xStr_IP, xWORD_Port, xStr_CandidateType, xStr_TcpCandidateType, xStr_RelatedAddress, xWORD_RelatedPort, xStr_RemoteIP, xStr_LastUsed, xPUID_TransportObjectID, xPUID_HostPortObjectID, xPUID_RelayPortObjectID, xPUID_TCPPortObjectID) {}
#define EventWriteOrtcIceGathererInstallQuickRoute(xStr_Method, xPUID, xPtr_Candidate, xStr_IPAddress, xPUID_RouteID) {}
#define EventWriteOrtcIceGathererRemoveQuickRoute(xStr_Method, xPUID, xPtr_Candidate, xStr_IPAddress, xPUID_RouteID) {}

#define EventWriteOrtcIceGathererInternalWakeEventFired(xStr_Method, xPUID) {}
#define EventWriteOrtcIceGathererInternalDnsLoookupCompleteEventFired(xStr_Method, xPUID, xPUID_QueryID) {}
#define EventWriteOrtcIceGathererInternalTimerEventFired(xStr_Method, xPUID, xPUID_TimerID, xStr_TimerType, xPUID_RelatedObjectID) {}
#define EventWriteOrtcIceGathererInternalLikelyReflexiveActivityEventFired(xStr_Method, xPUID, xPUID_RouteID, xPUID_xRouterRouteID) {}

#define EventWriteOrtcIceGathererInternalSocketReadReadyEventFired(xStr_Method, xPUID, xRelatedObjectID) {}
#define EventWriteOrtcIceGathererInternalSocketWriteReadyEventFired(xStr_Method, xPUID, xRelatedObjectID) {}
#define EventWriteOrtcIceGathererInternalSocketExceptionEventFired(xStr_Method, xPUID, xRelatedObjectID) {}

#define EventWriteOrtcIceGathererInternalBackOffTimerStateChangedEventFired(xStr_Method, xPUID, xPUID_BackOffTimerObjectID, xStr_State) {}

#define EventWriteOrtcIceGathererInternalStunDiscoverySendPacket(xStr_Method, xPUID, xPUID_StunDiscoveryObjectID, xStr_DestinationIP, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}
#define EventWriteOrtcIceGathererInternalStunDiscoveryCompleteEventFired(xStr_Method, xPUID, xPUID_StunDiscoveryObjectID, xPUID_HostPortObjectID, xStr_MappedIP) {}

#define EventWriteOrtcIceGathererInternalTurnSocketStateChangeEventFired(xStr_Method, xPUID, xPUID_TurnSocketObjectID, xStr_State) {}

#define EventWriteOrtcIceGathererResolveHostIP(xStr_Method, xPUID, xPUID_DnsQueryID, xStr_HostName) {}
#define EventWriteOrtcIceGathererResolveFoundHostIP(xStr_Method, xPUID, xStr_HostIP, xStr_HostName, xStr_InterfaceName, xULONG_AdapterSpecific) {}

#define EventWriteOrtcIceGathererPendingResolveHostIP(xStr_Method, xPUID, xStr_ProfileName, xStr_HostName, xStr_IP) {}

#define EventWriteOrtcIceGathererHostPortCreate(xStr_Method, xPUID, xPUID_HostPortObjectID, xStr_HostIP) {}
#define EventWriteOrtcIceGathererHostPortDestroy(xStr_Method, xPUID, xPUID_HostPortObjectID, xStr_HostIP) {}
#define EventWriteOrtcIceGathererHostPortBind(xStr_Method, xPUID, xPUID_HostPortObjectID, xStr_HostIP, Str_ProtocolType, xBool_Success) {}
#define EventWriteOrtcIceGathererHostPortClose(xStr_Method, xPUID, xPUID_HostPortObjectID, xStr_HostIP, Str_ProtocolType) {}

#define EventWriteOrtcIceGathererReflexivePortCreate(xStr_Method, xPUID, xPUID_ReflexivePortObjectID, xStr_Server) {}
#define EventWriteOrtcIceGathererReflexivePortDestroy(xStr_Method, xPUID, xPUID_ReflexivePortObjectID, xStr_Server) {}
#define EventWriteOrtcIceGathererReflexivePortFoundMapped(xStr_Method, xPUID, xPUID_ReflexivePortObjectID, xStr_MappedIP) {}

#define EventWriteOrtcIceGathererRelayPortCreate(xStr_Method, xPUID, xPUID_RelayPortObjectID, xStr_Server, xStr_Username, xStr_Credential, xStr_CredentialType) {}
#define EventWriteOrtcIceGathererRelayPortDestroy(xStr_Method, xPUID, xPUID_RelayPortObjectID, xStr_Server) {}
#define EventWriteOrtcIceGathererRelayPortFoundIP(xStr_Method, xPUID, xPUID_RelayPortObjectID, xStr_CandidateType, xStr_IP) {}

#define EventWriteOrtcIceGathererTcpPortCreate(xStr_Method, xPUID, xPUID_TcpPortObjectID, xStr_RemoteIP) {}
#define EventWriteOrtcIceGathererTcpPortDestroy(xStr_Method, xPUID, xPUID_TcpPortObjectID, xStr_RemoteIP) {}

#define EventWriteOrtcIceGathererUdpSocketPacketReceivedFrom(xStr_Method, xPUID, xStr_FromIP, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}
#define EventWriteOrtcIceGathererUdpSocketPacketForwardingToTurnSocket(xStr_Method, xPUID, xStr_FromIP, xBool_IsStun, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}
#define EventWriteOrtcIceGathererUdpSocketPacketSentTo(xStr_Method, xPUID, xStr_BoundIP, xStr_ToIP, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}

#define EventWriteOrtcIceGathererTcpSocketPacketReceivedFrom(xStr_Method, xPUID, xStr_FromIP, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}
#define EventWriteOrtcIceGathererTcpSocketSentOutgoing(xStr_Method, xPUID, xStr_FromIP, xPtr_Buffer, xsize_t_BufferSizeInBytes) {}

#else

// duplicate testing compilation methods used to verify compilation when macros get defined
namespace ortc
{

inline void EventWriteOrtcCreate(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcDestroy(const char *xStr_Method, PUID xPUID) {}

inline void EventWriteOrtcCertificateCreate(const char *xStr_Method, PUID xPUID, const char *xStr_KeyGenAlgorithm, const char *xStr_Name, const char *xStr_NamedCurve, size_t xsize_t_KeyLength, size_t xsize_t_RandomBits, const char *xStr_PublicExponenetLength, long long xlong_long_LifetimeInSeconds, long long xlong_long_NotBeforeWindowInSeconds, const char *xStr_Expires) {}
inline void EventWriteOrtcCertificateDestroy(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcCertificateGenerated(const char *xStr_Method, PUID xPUID, bool xBool_Success) {}
inline void EventWriteOrtcCertificateFingerprint(const char *xStr_Method, PUID xPUID, const char *xStr_FingerprintAlgorithm, const char *xStr_FingerprintValue) {}

inline void EventWriteOrtcDataChannelCreate(const char *xStr_Method, PUID xPUID, PUID xPUID_DataTransport, const char *xStr_Parameters, bool xBool_Incoming, WORD xWORD_SessionID) {}
inline void EventWriteOrtcDataChannelDestroy(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcDataChannelClose(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcDataChannelCancel(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcDataChannelStep(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcDataChannelStateChangedEventFired(const char *xStr_Method, PUID xPUID, const char *xStr_State) {}
inline void EventWriteOrtcDataChannelErrorEventFired(const char *xStr_Method, PUID xPUID, WORD xWORD_Error, const char *xStr_ErrorReason) {}

inline void EventWriteOrtcDataChannelSendControlOpen(const char *xStr_Method, PUID xPUID, BYTE xBYTE_MessageType, BYTE xBYTE_ChannelType, WORD xWORD_Priority, DWORD xDWORD_ReliabilityParameter, WORD xWORD_LabelLength, WORD xWORD_ProtocolLength, const char *xStr_Label, const char *xStr_Protocol) {}
inline void EventWriteOrtcDataChannelSendControlAck(const char *xStr_Method, PUID xPUID, BYTE xBYTE_MessageType) {}

inline void EventWriteOrtcDataChannelBufferedAmountLowThresholdChanged(const char *xStr_Method, PUID xPUID, size_t xsize_t_NewThreshold, size_t xsize_t_mOldThreshold, size_t xsize_t_OutgoingBufferFillSize, bool xBool_BufferedAmountLowThresholdFired) {}

inline void EventWriteOrtcDataChannelSendString(const char *xStr_Method, PUID xPUID, const char *xStr_Data) {}
inline void EventWriteOrtcDataChannelSendBinary(const char *xStr_Method, PUID xPUID, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcDataChannelOutgoingBufferPacket(const char *xStr_Method, PUID xPUID, unsigned int xUInt_Type, WORD xWORD_SessionID, bool xBool_Ordered, long long xlong_long_MaxPacketLifetimeInMilliseconds, bool xBool_HasMaxRetransmits, DWORD xDWORD_MaxRetransmits, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcDataChannelOutgoingBufferPacketDelivered(const char *xStr_Method, PUID xPUID, unsigned int xUInt_Type, WORD xWORD_SessionID, bool xBool_Ordered, long long xlong_long_MaxPacketLifetimeInMilliseconds, bool xBool_HasMaxRetransmits, DWORD xDWORD_MaxRetransmits, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcDataChannelReceivedControlOpen(const char *xStr_Method, PUID xPUID, bool xBool_Incoming, BYTE xBYTE_MessageType, BYTE xBYTE_ChannelType, WORD xWORD_Priority, DWORD xDWORD_ReliabilityParameter, WORD xWORD_LabelLength, WORD xWORD_ProtocolLength, const char *xStr_Label, const char *xStr_Protocol) {}
inline void EventWriteOrtcDataChannelReceivedControlAck(const char *xStr_Method, PUID xPUID, BYTE xBYTE_MessageType) {}

inline void EventWriteOrtcDataChannelMessageFiredEvent(const char *xStr_Method, PUID xPUID, unsigned int xUInt_Type, WORD xWORD_SessionID, WORD xWORD_SequenceNumber, DWORD xDWORD_Timestamp, int xInt_Flags, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcDataChannelSCTPTransportDeliverOutgoingPacket(const char *xStr_Method, PUID xPUID, unsigned int xUInt_Type, WORD xWORD_SessionID, bool xBool_Ordered, long long xlong_long_MaxPacketLifetimeInMilliseconds, bool xBool_HasMaxRetransmits, DWORD xDWORD_MaxRetransmits, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcDataChannelSCTPTransportRequestShutdown(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcDataChannelSCTPTransportRequestNotifyClosed(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcDataChannelSCTPTransportStateChanged(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcDataChannelSCTPTransportSendReady(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcDataChannelSCTPTransportSendReadyFailure(const char *xStr_Method, PUID xPUID, WORD xWORD_Error, const char *xStr_ErrorReason) {}
inline void EventWriteOrtcDataChannelSCTPTransportReceivedIncomingPacket(const char *xStr_Method, PUID xPUID, unsigned int xUInt_Type, WORD xWORD_SessionID, WORD xWORD_SequenceNumber, DWORD xDWORD_Timestamp, int xInt_Flags, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcDtlsTransportCreate(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransport, const char *xStr_Component, size_t xsize_t_MaxPendingDTLSBuffer, size_t xsize_t_MaxPendingRTPPackets, size_t xsize_t_TotalCertificates) {}
inline void EventWriteOrtcDtlsTransportDestroy(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcDtlsTransportInitization(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransport, PUID xPUID_RtpListener, size_t xsize_t_TotalCiphers, size_t xsize_t_TotalFingerprints) {}
inline void EventWriteOrtcDtlsTransportStep(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcDtlsTransportCancel(const char *xStr_Method, PUID xPUID) {}

inline void EventWriteOrtcDtlsTransportInitizationInstallCipher(const char *xStr_Method, PUID xPUID, const char *xStr_Cipher) {}
inline void EventWriteOrtcDtlsTransportInitizationInstallFingerprint(const char *xStr_Method, PUID xPUID, const char *xStr_Algorithm, const char *xStr_Value, bool xBool_IsLocal) {}

inline void EventWriteOrtcDtlsTransportStart(const char *xStr_Method, PUID xPUID, const char *xStr_Role, size_t xsize_t_TotalFingerprints) {}
inline void EventWriteOrtcDtlsTransportStop(const char *xStr_Method, PUID xPUID) {}

inline void EventWriteOrtcDtlsTransportRoleSet(const char *xStr_Method, PUID xPUID, const char *xStr_Role) {}

inline void EventWriteOrtcDtlsTransportReceivedPacket(const char *xStr_Method, PUID xPUID, unsigned int xUInt_ViaTransport, bool xBool_IsDtlsPacket, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcDtlsTransportReceivedStunPacket(const char *xStr_Method, PUID xPUID, unsigned int xUInt_ViaTransport) {}

inline void EventWriteOrtcDtlsTransportForwardingEncryptedPacketToSrtpTransport(const char *xStr_Method, PUID xPUID, PUID xPUID_SrtpTransportObjectID, unsigned int xUInt_ViaTransport, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcDtlsTransportForwardingPacketToDataTransport(const char *xStr_Method, PUID xPUID, PUID xPUID_DataTransportObjectID, unsigned int xUInt_ViaTransport, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcDtlsTransportForwardingPacketToRtpListener(const char *xStr_Method, PUID xPUID, PUID xPUID_RtpListenerObjectID, unsigned int xUInt_ViaTransport,  unsigned int xUInt_PacketType, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcDtlsTransportSendRtpPacket(const char *xStr_Method, PUID xPUID, unsigned int xUInt_SendOverComponent, unsigned int xUInt_PacketType, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcDtlsTransportSendDataPacket(const char *xStr_Method, PUID xPUID, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcDtlsTransportForwardDataPacketToIceTransport(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcDtlsTransportSendEncryptedRtpPacket(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID, unsigned int xUInt_SendOverComponent, unsigned int xUInt_PacketType, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcDtlsTransportInternalTimerEventFired(const char *xStr_Method, PUID xPUID, PUID xPUID_TimerID) {}
inline void EventWriteOrtcDtlsTransportInternalIceStateChangeEventFired(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID, const char *xStr_State) {}
inline void EventWriteOrtcDtlsTransportInternalSrtpTransportLifetimeRemainingEventFired(const char *xStr_Method, PUID xPUID, PUID xPUID_SrtpTransportObjectID, ULONG xULONG_LeastLifetimeRemainingPercentageForAllKeys, ULONG xULONG_OverallLifetimeRemainingPercentage) {}

inline void EventWriteOrtcDtlsTransportStateChangedEventFired(const char *xStr_Method, PUID xPUID, const char *xStr_State) {}
inline void EventWriteOrtcDtlsTransportErrorEventFired(const char *xStr_Method, PUID xPUID, WORD xWORD_Error, const char *xStr_ErrorReason) {}

inline void EventWriteOrtcDtlsTransportSrtpKeyingMaterialSetup(const char *xStr_Method, PUID xPUID, PUID xPUID_SecureTransportObjectID, const char *xStr_Direction, const char *xStr_Cipher, const void *xPtr_KeyingMaterialBuffer, size_t xsize_t_KeyingMaterialBufferSizeInBytes) {}

inline void EventWriteOrtcIceGathererCreate(const char *xStr_Method, PUID xPUID, PUID xPUID_GethererRouterObjectID, const char *xStr_UsernameFragment, const char *xStr_Password, bool xBool_CreateTCPCandidates, bool xBool_ContinousGathering, size_t xsize_t_TotalInterfacePolicies, size_t xsize_t_TotalIceServers, long long xlong_long_ReflexiveInactivityTimeInSeconds, long long xlong_long_RelayInactivityTimeInSeoncds, long long xlong_long_MaxBufferingTimeInSeconds, long long xlong_long_RecheckIPsInSeconds, size_t xsize_t_MaxTotalBuffers, size_t xsize_t_MaxTCPBufferingSizePendingConnectionInBytes, size_t xsize_t_MaxTCPBufferingSizeConnectedInBytes, bool xBool_GatherPassiveTCP) {}
inline void EventWriteOrtcIceGathererDestroy(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcIceGathererStep(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcIceGathererCancel(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcIceGathererCreatedAssociateGatherer(const char *xStr_Method, PUID xPUID, PUID xPUID_AssociatedGathererObjectID) {}

inline void EventWriteOrtcIceGathererInitializeInstallInterfaceNameMapping(const char *xStr_Method, PUID xPUID, const char *xStr_InterfaceNameRegularExpresssion, const char *xStr_InterfaceType, ULONG xULONG_OrderIndex) {}
inline void EventWriteOrtcIceGathererInitializeInstallPreference(const char *xStr_Method, PUID xPUID, const char *xStr_PreferenceType, const char *xStr_Preference, const char *xStr_SubPreference, DWORD xDWORD_PreferenceValue) {}

inline void EventWriteOrtcIceGathererGather(const char *xStr_Method, PUID xPUID, bool xBool_ContinuousGathering, size_t xsize_t_TotalInterfacePolicies, size_t xsize_t_TotalIceServers) {}

inline void EventWriteOrtcIceGathererStateChangedEventFired(const char *xStr_Method, PUID xPUID, const char *xStr_State) {}
inline void EventWriteOrtcIceGathererErrorEventFired(const char *xStr_Method, PUID xPUID, WORD xWORD_Error, const char *xStr_Reason) {}

inline void EventWriteOrtcIceGathererAddCandidateEventFired(const char *xStr_Method, PUID xPUID, const char *xStr_LocalHash, const char *xStr_NotifyHash, const char *xStr_InterfaceType, const char *xStr_Foundation, DWORD xDWORD_Priority, DWORD xDWORD_UnfreezePriority, const char *xStr_Protocol, const char *xStr_IP, WORD xWORD_Port, const char *xStr_CandidateType, const char *xStr_TcpType, const char *xStr_RelatedAddress, WORD xWORD_RelatedPort) {}
inline void EventWriteOrtcIceGathererRemoveCandidateEventFired(const char *xStr_Method, PUID xPUID, const char *xStr_LocalHash, const char *xStr_NotifyHash, const char *xStr_InterfaceType, const char *xStr_Foundation, DWORD xDWORD_Priority, DWORD xDWORD_UnfreezePriority, const char *xStr_Protocol, const char *xStr_IP, WORD xWORD_Port, const char *xStr_CandidateType, const char *xStr_TcpType, const char *xStr_RelatedAddress, WORD xWORD_RelatedPort) {}
inline void EventWriteOrtcIceGathererFilterCandidateEventFired(const char *xStr_Method, PUID xPUID,  unsigned int xUInt_FilterPolicy, const char *xStr_BoundIP, const char *xStr_InterfaceType, const char *xStr_Foundation, DWORD xDWORD_Priority, DWORD xDWORD_UnfreezePriority, const char *xStr_Protocol, const char *xStr_IP, WORD xWORD_Port, const char *xStr_CandidateType, const char *xStr_TcpType, const char *xStr_RelatedAddress, WORD xWORD_RelatedPort) {}

inline void EventWriteOrtcIceGathererInstallIceTransport(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID, const char *xStr_RemoteUsernameFragment) {}
inline void EventWriteOrtcIceGathererRemoveIceTransport(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID) {}
inline void EventWriteOrtcIceGathererInternalIceTransportStateChangedEventFired(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID) {}

inline void EventWriteOrtcIceGathererRemovetAllIceTransportRelatedRoutes(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID) {}
inline void EventWriteOrtcIceGathererSendIceTransportPacket(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID, PUID xPUID_RouteID, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcIceGathererSendIceTransportPacketFailed(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID, PUID xPUID_RouteID, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcIceGathererSendIceTransportPacketViaUdp(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID, PUID xPUID_RouteID, PUID xPUID_HostPortObjectID, const char *xStr_RemoteIP, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcIceGathererSendIceTransportPacketViaTcp(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID, PUID xPUID_RouteID, PUID xPUID_TcpPortObjectID, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcIceGathererSendIceTransportPacketViaTurn(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID, PUID xPUID_RouteID, PUID xPUID_TurnObjectID, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcIceGathererDeliverIceTransportIncomingPacket(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID, PUID xPUID_RouteID, PUID xPUID_xRouterRouteID, bool xBool_WasBuffered, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcIceGathererBufferIceTransportIncomingPacket(const char *xStr_Method, PUID xPUID, PUID xPUID_xRouterRouteID, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcIceGathererDeliverIceTransportIncomingStunPacket(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID, PUID xPUID_RouteID, PUID xPUID_xRouterRouteID, bool xBool_WasBuffered) {}
inline void EventWriteOrtcIceGathererBufferIceTransportIncomingStunPacket(const char *xStr_Method, PUID xPUID, PUID xPUID_xRouterRouteID) {}
inline void EventWriteOrtcIceGathererErrorIceTransportIncomingStunPacket(const char *xStr_Method, PUID xPUID, PUID xPUID_IceTransportObjectID, PUID xPUID_RouteID, PUID xPUID_xRouterRouteID) {}

inline void EventWriteOrtcIceGathererDisposeBufferedIceTransportIncomingPacket(const char *xStr_Method, PUID xPUID, PUID xPUID_xRouterRouteID, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcIceGathererDisposeBufferedIceTransportIncomingStunPacket(const char *xStr_Method, PUID xPUID, PUID xPUID_xRouterRouteID) {}

inline void EventWriteOrtcIceGathererTurnSocketReceivedPacket(const char *xStr_Method, PUID xPUID, PUID xPUID_TurnSocketObjectID, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcIceGathererTurnSocketSendPacket(const char *xStr_Method, PUID xPUID, PUID xPUID_TurnSocketObjectID, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcIceGathererInstallRoute(const char *xStr_Method, PUID xPUID, PUID xPUID_RouteID, const char *xStr_InterfaceType, const char *xStr_Foundation, DWORD xDWORD_Priority, DWORD xDWORD_UnfreezePriority, const char *xStr_Protocol, const char *xStr_IP, WORD xWORD_Port, const char *xStr_CandidateType, const char *xStr_TcpCandidateType, const char *xStr_RelatedAddress, WORD xWORD_RelatedPort, const char *xStr_RemoteIP, const char *xStr_LastUsed, PUID xPUID_TransportObjectID, PUID xPUID_HostPortObjectID, PUID xPUID_RelayPortObjectID, PUID xPUID_TCPPortObjectID) {}
inline void EventWriteOrtcIceGathererRemoveRoute(const char *xStr_Method, PUID xPUID, PUID xPUID_RouteID, const char *xStr_InterfaceType, const char *xStr_Foundation, DWORD xDWORD_Priority, DWORD xDWORD_UnfreezePriority, const char *xStr_Protocol, const char *xStr_IP, WORD xWORD_Port, const char *xStr_CandidateType, const char *xStr_TcpCandidateType, const char *xStr_RelatedAddress, WORD xWORD_RelatedPort, const char *xStr_RemoteIP, const char *xStr_LastUsed, PUID xPUID_TransportObjectID, PUID xPUID_HostPortObjectID, PUID xPUID_RelayPortObjectID, PUID xPUID_TCPPortObjectID) {}
inline void EventWriteOrtcIceGathererInstallQuickRoute(const char *xStr_Method, PUID xPUID, const void *xPtr_Candidate, const char *xStr_IPAddress, PUID xPUID_RouteID) {}
inline void EventWriteOrtcIceGathererRemoveQuickRoute(const char *xStr_Method, PUID xPUID, const void *xPtr_Candidate, const char *xStr_IPAddress, PUID xPUID_RouteID) {}

inline void EventWriteOrtcIceGathererInternalWakeEventFired(const char *xStr_Method, PUID xPUID) {}
inline void EventWriteOrtcIceGathererInternalDnsLoookupCompleteEventFired(const char *xStr_Method, PUID xPUID, PUID xPUID_QueryID) {}
inline void EventWriteOrtcIceGathererInternalTimerEventFired(const char *xStr_Method, PUID xPUID, PUID xPUID_TimerID, const char *xStr_TimerType, PUID xPUID_RelatedObjectID) {}
inline void EventWriteOrtcIceGathererInternalLikelyReflexiveActivityEventFired(const char *xStr_Method, PUID xPUID, PUID xPUID_RouteID, PUID xPUID_xRouterRouteID) {}

inline void EventWriteOrtcIceGathererInternalSocketReadReadyEventFired(const char *xStr_Method, PUID xPUID, PUID xRelatedObjectID) {}
inline void EventWriteOrtcIceGathererInternalSocketWriteReadyEventFired(const char *xStr_Method, PUID xPUID, PUID xRelatedObjectID) {}
inline void EventWriteOrtcIceGathererInternalSocketExceptionEventFired(const char *xStr_Method, PUID xPUID, PUID xRelatedObjectID) {}

inline void EventWriteOrtcIceGathererInternalBackOffTimerStateChangedEventFired(const char *xStr_Method, PUID xPUID, PUID xPUID_BackOffTimerObjectID, const char *xStr_State) {}

inline void EventWriteOrtcIceGathererInternalStunDiscoverySendPacket(const char *xStr_Method, PUID xPUID, PUID xPUID_StunDiscoveryObjectID, const char *xStr_DestinationIP, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcIceGathererInternalStunDiscoveryCompleteEventFired(const char *xStr_Method, PUID xPUID, PUID xPUID_StunDiscoveryObjectID, PUID xPUID_HostPortObjectID, const char *xStr_MappedIP) {}

inline void EventWriteOrtcIceGathererInternalTurnSocketStateChangeEventFired(const char *xStr_Method, PUID xPUID, PUID xPUID_TurnSocketObjectID, const char *xStr_State) {}

inline void EventWriteOrtcIceGathererResolveHostIP(const char *xStr_Method, PUID xPUID, PUID xPUID_DnsQueryID, const char *xStr_HostName) {}
inline void EventWriteOrtcIceGathererResolveFoundHostIP(const char *xStr_Method, PUID xPUID, const char *xStr_HostIP, const char *xStr_HostName, const char *xStr_InterfaceName, ULONG xULONG_AdapterSpecific) {}

inline void EventWriteOrtcIceGathererPendingResolveHostIP(const char *xStr_Method, PUID xPUID, const char *xStr_ProfileName, const char *xStr_HostName, const char *xStr_IP) {}

inline void EventWriteOrtcIceGathererHostPortCreate(const char *xStr_Method, PUID xPUID, PUID xPUID_HostPortObjectID, const char *xStr_HostIP) {}
inline void EventWriteOrtcIceGathererHostPortDestroy(const char *xStr_Method, PUID xPUID, PUID xPUID_HostPortObjectID, const char *xStr_HostIP) {}
inline void EventWriteOrtcIceGathererHostPortBind(const char *xStr_Method, PUID xPUID, PUID xPUID_HostPortObjectID, const char *xStr_HostIP, const char *Str_ProtocolType, bool xBool_Success) {}
inline void EventWriteOrtcIceGathererHostPortClose(const char *xStr_Method, PUID xPUID, PUID xPUID_HostPortObjectID, const char *xStr_HostIP, const char *Str_ProtocolType) {}

inline void EventWriteOrtcIceGathererReflexivePortCreate(const char *xStr_Method, PUID xPUID, PUID xPUID_ReflexivePortObjectID, const char *xStr_Server) {}
inline void EventWriteOrtcIceGathererReflexivePortDestroy(const char *xStr_Method, PUID xPUID, PUID xPUID_ReflexivePortObjectID, const char *xStr_Server) {}
inline void EventWriteOrtcIceGathererReflexivePortFoundMapped(const char *xStr_Method, PUID xPUID, PUID xPUID_ReflexivePortObjectID, const char *xStr_MappedIP) {}

inline void EventWriteOrtcIceGathererRelayPortCreate(const char *xStr_Method, PUID xPUID, PUID xPUID_RelayPortObjectID, const char *xStr_Server, const char *xStr_Username, const char *xStr_Credential, const char *xStr_CredentialType) {}
inline void EventWriteOrtcIceGathererRelayPortDestroy(const char *xStr_Method, PUID xPUID, PUID xPUID_RelayPortObjectID, const char *xStr_Server) {}
inline void EventWriteOrtcIceGathererRelayPortFoundIP(const char *xStr_Method, PUID xPUID, PUID xPUID_RelayPortObjectID, const char *xStr_CandidateType, const char *xStr_IP) {}

inline void EventWriteOrtcIceGathererTcpPortCreate(const char *xStr_Method, PUID xPUID, PUID xPUID_TcpPortObjectID, const char *xStr_RemoteIP) {}
inline void EventWriteOrtcIceGathererTcpPortDestroy(const char *xStr_Method, PUID xPUID, PUID xPUID_TcpPortObjectID, const char *xStr_RemoteIP) {}

inline void EventWriteOrtcIceGathererUdpSocketPacketReceivedFrom(const char *xStr_Method, PUID xPUID, const char *xStr_FromIP, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcIceGathererUdpSocketPacketForwardingToTurnSocket(const char *xStr_Method, PUID xPUID, const char *xStr_FromIP, bool xBool_IsStun, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcIceGathererUdpSocketPacketSentTo(const char *xStr_Method, PUID xPUID, const char *xStr_BoundIP, const char *xStr_ToIP, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

inline void EventWriteOrtcIceGathererTcpSocketPacketReceivedFrom(const char *xStr_Method, PUID xPUID, const char *xStr_FromIP, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}
inline void EventWriteOrtcIceGathererTcpSocketSentOutgoing(const char *xStr_Method, PUID xPUID, const char *xStr_FromIP, const void *xPtr_Buffer, size_t xsize_t_BufferSizeInBytes) {}

}
#endif //ndef ORTC_USE_NOOP_EVENT_TRACE_MACROS

#endif //USE_ETW
