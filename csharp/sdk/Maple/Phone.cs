﻿using HidSharp;
using HidSharp.Reports;
using HidSharp.Reports.Input;
using System;
using System.Linq;
using System.Threading;

namespace Maple
{
    public class Phone : IDisposable
    {
        public Version SoftwareVersion { get; private set; }
        public Version HardwareVersion { get { return hiddev.ReleaseNumber; } }

        private AudioRouter router;

        private HidDevice hiddev { get { return stream.Device; } }
        private HidStream stream;

        private Report verReport;
        private Report txReport;
        private HidDeviceInputReceiver inputReceiver;
        private DeviceItemInputParser inputParser;

        private bool _LoopState = false;
        private bool _OffHook = false;

        private bool IsRequestPending = false;
        public bool LoopState { get { return this._LoopState; } }
        public bool OffHook { get { return this._OffHook; } }

        public event Action<Phone, bool> RingingSignal = delegate { };
        public event Action<Phone, bool> LoopPresence = delegate { };
        public event Action<Phone, bool> RemoteOffHook = delegate { };
        public event Action<Phone, bool> Polarity = delegate { };

        protected Phone(HidStream hidStream)
        {
            this.stream = hidStream;
            this.router = new AudioRouter();

            var reportDescriptor = hiddev.GetReportDescriptor();
            var deviceItem = reportDescriptor.DeviceItems.First();
            var freports = deviceItem.FeatureReports;
            var oreports = deviceItem.OutputReports;
            var ireports = deviceItem.InputReports;

            this.txReport = oreports.First();
            this.verReport = freports.First();

            var buffer = verReport.CreateBuffer();
            stream.GetFeature(buffer);
            int major = 0, minor = 0, rev = 0;
            verReport.Read(buffer, 0, (dataValue) =>
            {
                var usage = (HidUsage.GenericDevice)dataValue.Usages.FirstOrDefault();
                int val = dataValue.GetLogicalValue();
                switch (usage)
                {
                    case HidUsage.GenericDevice.Major:
                        major = val;
                        break;
                    case HidUsage.GenericDevice.Minor:
                        minor = val;
                        break;
                    case HidUsage.GenericDevice.Revision:
                        rev = val;
                        break;
                    default:
                        break;
                }
            });
            SoftwareVersion = new Version(major, minor, rev);

            this.inputReceiver = reportDescriptor.CreateHidDeviceInputReceiver();
            this.inputReceiver.Received += InputHandler;
            this.inputReceiver.Start(hidStream);
            this.inputParser = deviceItem.CreateDeviceItemInputParser();

            SendControl(true);
        }

        public static bool TryOpen(HidDevice hiddev, out Phone MaplePhoneControl)
        {
            MaplePhoneControl = null;
            if (!hiddev.IsMapleControlDevice())
                return false;
            if (hiddev.GetTopLevelUsage() != (uint)HidUsage.Telephony.DualModePhone)
                return false;

            try
            {
                HidStream hidStream;
                if (!hiddev.TryOpen(out hidStream))
                    return false;
                hidStream.ReadTimeout = Timeout.Infinite;
                MaplePhoneControl = new Phone(hidStream);
            }
            catch (Exception err)
            {
                Console.WriteLine("ERROR: " + err.ToString());
                return false;
            }
            return true;
        }

        public static Phone First()
        {
            Phone r = null;
            var device = DeviceList.Local.GetHidDevices().First(d => Phone.TryOpen(d, out r));
            return r;
        }

        public void Dispose()
        {
            if (OffHook)
            {
                HangUp();
            }
            this.inputReceiver.Received -= InputHandler;
            router.Dispose();
            stream.Dispose();
        }

        private void InputHandler(object sender, EventArgs e)
        {
            Console.WriteLine(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
            Console.WriteLine("Input Handler Called!");
            var inputReportBuffer = new byte[hiddev.GetMaxInputReportLength()];
            Report report;
            while (inputReceiver.TryRead(inputReportBuffer, 0, out report))
            {
                // Console.WriteLine("report: " + report.ToString());
                // Parse the report if possible.
                // This will return false if (for example) the report applies to a different DeviceItem.
                if (inputParser.TryParseReport(inputReportBuffer, 0, report))
                {
                    bool ringingChanged = false;
                    bool offHookChanged = false;
                    bool loopStateChanged = false;
                    bool isRinging = false;

                    while (inputParser.HasChanged)
                    {
                        int changedIndex = inputParser.GetNextChangedIndex();
                        var previousDataValue = inputParser.GetPreviousValue(changedIndex);
                        var dataValue = inputParser.GetValue(changedIndex);

                        // Console.WriteLine("Changed Index: " + changedIndex);
                        // Console.WriteLine("previouseDataValue: " + previousDataValue);
                        // Console.WriteLine("dataValue: " + dataValue);

                        var usages = (HidUsage.Telephony)dataValue.Usages.FirstOrDefault();
                        switch (usages)
                        {
                            case HidUsage.Telephony.HookSwitch:
                                _OffHook = Convert.ToBoolean(dataValue.GetLogicalValue());
                                offHookChanged = true;
                                break;
                            case HidUsage.Telephony.AlternateFunction:
                                var polarity = Convert.ToBoolean(dataValue.GetLogicalValue());
                                Polarity(this, polarity);
                                Console.WriteLine("POLARITY? " + polarity);
                                break;
                            case HidUsage.Telephony.RingEnable:
                                ringingChanged = true;
                                isRinging = Convert.ToBoolean(dataValue.GetLogicalValue());
                                break;
                            case HidUsage.Telephony.HostControl:
                                _LoopState = Convert.ToBoolean(dataValue.GetLogicalValue());
                                loopStateChanged = true;
                                break;
                            default:
                                Console.WriteLine("Unhandled INPUT Event:" + usages);
                                break;
                        }
                    }

                    // Accumulate all state changes in this payload, then notify subscribers
                    // after the state has been updated.
                    if (ringingChanged)
                    {
                        Console.WriteLine("Ringing state changed: " + isRinging);
                        RingingSignal(this, isRinging);
                    }
                    if (loopStateChanged)
                    {
                        Console.WriteLine("LoopState changed: " + LoopState);
                        LoopPresence(this, LoopState);
                    }
                    if (offHookChanged)
                    {
                        Console.WriteLine("OffHook changed: " + OffHook);
                        RemoteOffHook(this, OffHook);
                    }
                }
            }
            IsRequestPending = false;
            Console.WriteLine("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        }

        /**
         * Take the receiver Off Hook, if there is a valid line signal detected.
         */
        public bool TakeOffHook()
        {
            Console.WriteLine("Internal - Attempt to Take OffHook");
            // Never take the phone OFF_HOOK unless LOOP detect indicates a valid line is attached
            if (LoopState)
            {
                Console.WriteLine("Internal - Taking OffHook");
                // Attempt to take it off hook now
                SendControl(true, true);
                Console.WriteLine("Waiting for OffHook Notification");
                WaitForOffHook();

                // Now that we're off hook, wire up for sound.
                router.Start();
                Console.WriteLine("OffHook Notification Received, and sound connected");
            }
            else
            {
                Console.WriteLine("Cannot take off hook without valid line detected.");
            }

            return OffHook;
        }

        /**
         * Place the receiver back "On Hook"
         */
        public void HangUp()
        {
            SendControl(true, false);
            WaitForResponse();
        }

        private void WaitForOffHook()
        {
            if (!OffHook)
            {
                Console.WriteLine("Waiting for OffHook"); 
            }
            while (!OffHook)
            {
                // Wait for firmware confirmation that the line is Off Hook.
                // TODO(lbayes): Add tone detection for open line dial tone.
                Thread.Sleep(TimeSpan.FromMilliseconds(10));
                // TODO(lbayes): Add a timeout to avoid blocking forever.
            }

        }

        private void WaitForResponse()
        {
            if (IsRequestPending)
            {
                Console.WriteLine("Waiting for a response");
            }
            while (IsRequestPending)
            {
                Thread.Sleep(TimeSpan.FromMilliseconds(10));
                // TODO(lbayes): Add a timeout to avoid blocking forever.
            }
        }

        public bool Dial(String phoneNumbers)
        {
            phoneNumbers = phoneNumbers.Replace("(", "")
                                       .Replace(")", "")
                                       .Replace(" ", "")
                                       .Replace("-", "");

            while(IsRequestPending)
            {
                Thread.Sleep(TimeSpan.FromMilliseconds(10));
                // TODO(lbayes): Add a timeout.
            }

            if (!LoopState)
            {
                Console.WriteLine("Cannot Dial without a valid line detected.");
                return false;
            }

            if (!OffHook)
            {
                Console.WriteLine("Taking OffHook");
                TakeOffHook();
                // TODO(lbayes): Instead, wait until a DTMF line open sound is detected on the expected
                // RX Device.
                Thread.Sleep(TimeSpan.FromSeconds(2));
            }

            // Send the DTMF codes through the open line.
            router.GenerateTones(phoneNumbers);
            return true;
        }

        public void SendControl(bool hostready, bool offHook = false)
        {
            if (IsRequestPending)
            {
                Console.WriteLine("THERE IS A REQUEST PENDING! Waiting for it complete");
                WaitForResponse();
            }

            IsRequestPending = true;
            var report = txReport;
            var buffer = report.CreateBuffer();

            report.Write(buffer, 0, (buf, bitOffset, dataItem, indexOfDataItem) =>
            {
                var usage = (HidUsage.Telephony)dataItem.Usages.GetAllValues().FirstOrDefault();
                switch (usage)
                {
                    case HidUsage.Telephony.HostAvailable:
                        dataItem.WriteRaw(buf, bitOffset, 0, Convert.ToUInt32(hostready));
                        break;
                    case HidUsage.Telephony.ActivateHandsetAudio:
                        Console.WriteLine("Sending offHook with: " + offHook);
                        dataItem.WriteRaw(buf, bitOffset, 0, Convert.ToUInt32(!offHook));
                        break;
                    default:
                        break;
                }
            });
            stream.Write(buffer);
        }
    }
}
