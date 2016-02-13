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

}
#endif //ndef ORTC_USE_NOOP_EVENT_TRACE_MACROS

#endif //USE_ETW
