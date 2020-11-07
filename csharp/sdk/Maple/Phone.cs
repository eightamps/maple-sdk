using HidSharp;
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

        private readonly AudioStitcher stitcher;
        private readonly Dtmf dtmf;

        private HidDevice hiddev { get { return stream.Device; } }
        private readonly HidStream stream;

        private readonly Report verReport;
        private readonly Report txReport;
        private readonly HidDeviceInputReceiver inputReceiver;
        private readonly DeviceItemInputParser inputParser;

        private bool IsRequestPending = false;
        public bool IsRinging { get; private set; } = false;
        public bool LoopState { get; private set; } = false;
        public bool OffHook { get; private set; } = false;
        public bool Polarity { get; private set; } = false;

        public event Action<Phone, bool> RingingChanged = delegate { };
        public event Action<Phone, bool> LoopStateChanged = delegate { };
        public event Action<Phone, bool> OffHookChanged = delegate { };
        public event Action<Phone, bool> PolarityChanged = delegate { };

        // public static readonly String PhoneCapture = "Microphone (Telephone Audio)";
        // public static readonly String PhoneRender = "Speakers (Telephone Audio)";
        // public static readonly String PhoneCapture = "Telephone Audio";
        // public static readonly String PhoneRender = "Telephone Audio";
        public static readonly String PhoneCapture = "USB Audio Device";
        public static readonly String PhoneRender = "USB Audio Device";

        protected Phone(HidStream hidStream)
        {
            this.stream = hidStream;
            this.stitcher = new AudioStitcher(PhoneCapture, PhoneRender);
            this.dtmf = new Dtmf();

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
            Console.WriteLine("Maple DISPOSE with OffHook state: " + OffHook);

            // disable telephone control (also hangs up)
            SendControl(false);

            this.inputReceiver.Received -= InputHandler;
            stitcher.Dispose();
            stream.Dispose();
        }

        private void InputHandler(object sender, EventArgs e)
        {
            var inputReportBuffer = new byte[hiddev.GetMaxInputReportLength()];
            Report report;
            while (inputReceiver.TryRead(inputReportBuffer, 0, out report))
            {
                // Console.WriteLine("report: " + report.ToString());
                // Parse the report if possible.
                // This will return false if (for example) the report applies to a different DeviceItem.
                if (inputParser.TryParseReport(inputReportBuffer, 0, report))
                {
                    bool _ringingChanged = false;
                    bool _offHookChanged = false;
                    bool _loopStateChanged = false;
                    bool _polarityChanged = false;

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
                                OffHook = Convert.ToBoolean(dataValue.GetLogicalValue());
                                _offHookChanged = true;
                                break;
                            case HidUsage.Telephony.AlternateFunction:
                                Polarity = Convert.ToBoolean(dataValue.GetLogicalValue());
                                _polarityChanged = true;
                                break;
                            case HidUsage.Telephony.RingEnable:
                                IsRinging = Convert.ToBoolean(dataValue.GetLogicalValue());
                                _ringingChanged = true;
                                break;
                            case HidUsage.Telephony.HostControl:
                                LoopState = Convert.ToBoolean(dataValue.GetLogicalValue());
                                _loopStateChanged = true;
                                break;
                            default:
                                Console.WriteLine("Unhandled INPUT Event:" + usages);
                                break;
                        }
                    }

                    // Accumulate all state changes in this payload, then notify subscribers
                    // after the state has been updated.
                    if (_ringingChanged)
                    {
                        Console.WriteLine("Ringing state changed: " + IsRinging);
                        RingingChanged(this, IsRinging);
                    }
                    if (_loopStateChanged)
                    {
                        Console.WriteLine("LoopState changed: " + LoopState);
                        LoopStateChanged(this, LoopState);
                    }
                    if (_offHookChanged)
                    {
                        Console.WriteLine("OffHook changed: " + OffHook);
                        SyncRouterToHookState();
                        // Only take off hook if line is also available.
                        OffHookChanged(this, OffHook);
                    }
                    if (_polarityChanged)
                    {
                        Console.WriteLine("Polarity changed: " + Polarity);
                        PolarityChanged(this, Polarity);
                    }
                }
            }
            IsRequestPending = false;
            Console.WriteLine("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        }

        private void SyncRouterToHookState()
        {
            if (OffHook && !stitcher.IsActive)
            {
                stitcher.Start();
            }
            else if (!OffHook && stitcher.IsActive)
            {
                stitcher.Stop();
            }
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
                Thread.Sleep(TimeSpan.FromMilliseconds(5));
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

        public bool Dial(String input)
        {
            // Ensure we're not already waiting for a response.
            WaitForResponse();

            if (!LoopState)
            {
                Console.WriteLine("Cannot Dial without a valid line detected.");
                return false;
            }

            if (!OffHook)
            {
                Console.WriteLine("Taking OffHook");
                TakeOffHook();
            }

            // Wait for the audio router to get everything wired up.
            while (!stitcher.IsActive)
            {
                Thread.Sleep(TimeSpan.FromMilliseconds(5));
            }

            // Give it another second to settle down.
            Thread.Sleep(TimeSpan.FromSeconds(3));

            // Send the DTMF codes through the open line.
            dtmf.GenerateDtmfTones(input, stitcher);
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
