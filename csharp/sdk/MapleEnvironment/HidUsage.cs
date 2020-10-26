using System;
using System.Collections.Generic;
using System.Text;

namespace HidUsage
{
    public enum Telephony : uint
    {
        Phone = 0x000B0001,
        DualModePhone = 0x000B014B,
        HookSwitch = 0x000B0020,
        AlternateFunction = 0x000B0029,
        RingEnable = 0x000B002D,
        HostControl = 0x000B00F0,
        HostAvailable,
        HostCallActive,
        ActivateHandsetAudio,
    }
    public enum GenericDevice : uint
    {
        BackgroundControls = 0x00060001,
        BatteryStrength = 0x00060020,
        WirelessChannel,
        WirelessID,
        DiscoverWirelessControl,
        SecurityCodeCharacterEntered,
        SecurityCodeCharacterErased,
        SecurityCodeCleared,
        SequenceID,
        SequenceIDReset,
        RFSignalStrength,
        SoftwareVersion,
        ProtocolVersion,
        HardwareVersion,
        Major,
        Minor,
        Revision,
        Handedness,
        EitherHand,
        LeftHand,
        RightHand,
        BothHands,
        GripPoseOffset = 0x00060040,
        PointerPoseOffset,
    }
    public enum EightAmps : uint
    {
        Switchy = 0xFF8A0001,
        Reddy = 0xFF8A0002,
        Envy = 0xFF8A0003,
        SwitchySelector = 0xFF8A0020,
        SwitchyAction = 0xFF8A0030,
        TagId = 0xFF8A0050,
        ReddyProtocolType = 0xFF8A0060,
        ReddyProtocolDataLength = 0xFF8A0061,
        ReddyProtocolData = 0xFF8A0062,
        ReddyStatus = 0xFF8A0063,
        ReddyDecodeTimeout = 0xFF8A0064,
        ReddyResetCmdReport = 0xFF8A0070,
        ReddyDecodeCmdReport = 0xFF8A0071,
        ReddyEncodeCmdReport = 0xFF8A0072,
        ReddyStatusRspReport = 0xFF8A0073,
        ReddyDecodeRspReport = 0xFF8A0074,

        HaGetCapabilitiesReport = 0xFF8A0080,
        HaGetTerminalCapabilitiesReport = 0xFF8A0081,
        HaGetTerminalStateReport = 0xFF8A0082,
        HaSetTerminalStateReport = 0xFF8A0083,
        HaInTerminalsCount = 0xFF8A0090,
        HaOutTerminalsCount = 0xFF8A0091,
        HaTerminalContactsCount = 0xFF8A0092,
        HaTerminalId = 0xFF8A0093,
        HaTerminalContactValue = 0xFF8A0094,
    }
}
