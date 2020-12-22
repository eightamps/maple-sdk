using HidSharp;
using HidSharp.Reports;
using HidSharp.Reports.Input;
using System;
using System.Linq;
using System.Threading;
using System.Timers;

namespace Maple
{
    enum RingState
    {
        Waiting,
        Ringing,
    }

    public class Phone : IDisposable
    {
        private const long RING_DURATION_MS = 1800;
        private const int USB_RESPONSE_TIMEOUT_MS = 50;
        public Version SoftwareVersion { get; private set; }
        public Version HardwareVersion { get { return hiddev.ReleaseNumber; } }

        private readonly AudioStitcher stitcher;
        private readonly Dtmf dtmf;
        private Thread DtmfThread;

        private HidDevice hiddev { get { return stream.Device; } }
        private readonly HidStream stream;
        private RingState ringState = RingState.Waiting;
        private System.Timers.Timer ringTimer;
        private System.Timers.Timer ringExitTimer;

        private readonly Report verReport;
        private readonly Report txReport;
        private readonly HidDeviceInputReceiver inputReceiver;
        private readonly DeviceItemInputParser inputParser;

        private bool IsRequestPending = false;
        public event Action<Phone, bool> RingingChanged = delegate { };
        public event Action<Phone, bool> LineIsAvailableChanged = delegate { };
        public event Action<Phone, bool> OffHookChanged = delegate { };
        public event Action<Phone, bool> PolarityChanged = delegate { };

        // public static readonly String PhoneCapture = "Microphone (Telephone Audio)";
        // public static readonly String PhoneRender = "Speakers (Telephone Audio)";
        public static readonly String PhoneCapture = "ASI Telephone";
        public static readonly String PhoneRender = "ASI Telephone";

        private System.Timers.Timer ResponseTimeoutTimer;

        public bool IsRinging {
            get
            {
                return ringState == RingState.Ringing;
            }
            private set
            {
                UpdateRingState(value);
            }
        }

        private bool lineIsAvailable;
        public bool LineIsAvailable {
            get
            {
                return lineIsAvailable;
            }
            private set
            {
                if (value != lineIsAvailable)
                {
                    lineIsAvailable = value;
                    Console.WriteLine("LineIsAvailable changed: " + lineIsAvailable);
                    LineIsAvailableChanged(this, lineIsAvailable);
                }
            }
        }

        private bool offHook;
        public bool OffHook
        {
            get
            {
                return offHook;
            }
            private set
            {
                if (value != offHook)
                {
                    offHook = value;
                    Console.WriteLine("OffHook changed: " + offHook);
                    SyncStitcherToHookState();
                    // Only take off hook if line is also available.
                    OffHookChanged(this, offHook);
                }
            }
        }
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

        private void UpdateRingState(bool ringSignal)
        {
            // Only enter the ring state if:
            // 1) The signal indicates RINGING state AND
            // 2) We were last in the Waiting state AND
            // 3) We are not currently waiting for a previous ring event to end.
            if (ringSignal && ringState == RingState.Waiting && ringTimer == null)
            {
                // We'll automatically exit Ringing state after RING_DURATION_MS,
                // so we can discard any off transitions from firmware.
                EnterRingingState();
            }
        }

        private void EnterRingingState()
        {
            ringTimer = new System.Timers.Timer(RING_DURATION_MS);
            ringTimer.Elapsed += OnRingTimerHandler;
            ringTimer.AutoReset = false;
            ringTimer.Enabled = true;
            ringTimer.Start();

            // Switch into Ringing state.
            ringState = RingState.Ringing;
            // Notify observers
            RingingChanged(this, IsRinging);
        }

        private void ExitRingingState()
        {
            ClearRingTimer();
            ringState = RingState.Waiting;
        }

        private void ClearRingTimer()
        {
            if (ringTimer != null)
            {
                ringTimer.Stop();
                ringTimer.Elapsed -= OnRingTimerHandler;
                ringTimer.Dispose();
                ringTimer = null;
            }
        }

        private void OnRingTimerHandler(object source, ElapsedEventArgs e)
        {
            ExitRingingState();
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

            this.inputReceiver.Received -= InputHandler;
            // disable telephone control (also hangs up)
            SendControl(false);

            if (ringTimer != null)
            {
                ClearRingTimer();
            }

            if (DtmfDialIsInProgress())
            {
                DtmfThread.Abort();
            }

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
                    while (inputParser.HasChanged)
                    {
                        int changedIndex = inputParser.GetNextChangedIndex();
                        // var previousDataValue = inputParser.GetPreviousValue(changedIndex);
                        var dataValue = inputParser.GetValue(changedIndex);

                        var usages = (HidUsage.Telephony)dataValue.Usages.FirstOrDefault();
                        switch (usages)
                        {
                            case HidUsage.Telephony.HookState:
                                OffHook = Convert.ToBoolean(dataValue.GetLogicalValue());
                                break;
                            case HidUsage.Telephony.RingState:
                                IsRinging = Convert.ToBoolean(dataValue.GetLogicalValue());
                                break;
                            case HidUsage.Telephony.LineIsAvailable:
                                LineIsAvailable = Convert.ToBoolean(dataValue.GetLogicalValue());
                                break;
                            default:
                                Console.WriteLine("Unhandled INPUT Event:" + usages);
                                break;
                        }
                    }
                }
            }
            StopResponseTimeoutIfRunning();
        }

        private void SyncStitcherToHookState()
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
            if (LineIsAvailable)
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
        public bool HangUp()
        {
            if (DtmfDialIsInProgress())
            {
                DtmfThread.Abort();
            }

            SendControl(true, false);
            WaitForResponse();
            return !OffHook;
        }

        private void WaitForOffHook()
        {
            if (!OffHook)
            {
                Console.WriteLine("Waiting for OffHook"); 
                while (!OffHook)
                {
                    // Wait for firmware confirmation that the line is Off Hook.
                    // TODO(lbayes): Add tone detection for open line dial tone.
                    Thread.Sleep(TimeSpan.FromMilliseconds(100));
                    // TODO(lbayes): Add a timeout to avoid blocking forever.
                }
            }
        }

        private void WaitForResponse()
        {
            if (IsRequestPending)
            {
                Console.WriteLine("Waiting for a response");
                while (IsRequestPending)
                {
                    Thread.Sleep(TimeSpan.FromMilliseconds(10));
                    // TODO(lbayes): Add a timeout to avoid blocking forever.
                }
            }
        }

        public bool Dial(String input)
        {
            if (!LineIsAvailable)
            {
                Console.WriteLine("Cannot Dial without a valid line detected.");
                return false;
            }

            // Ensure we're not already waiting for a response.
            WaitForResponse();

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

            // Dial using a separate thread with DTMF
            DtmfDial(input);
            return true;
        }
        private void DtmfDial(string input)
        {
            // If we're already waiting for tones, let them finish before 
            // we add more.
            if (DtmfDialIsInProgress())
            {
                DtmfThread.Join();
            }

            DtmfThread = new Thread(() =>
            {
                // Send the DTMF codes through the open line.
                dtmf.GenerateDtmfTones(input, stitcher);
            });
            DtmfThread.Start();
        }
        private bool DtmfDialIsInProgress()
        {
            return DtmfThread != null && DtmfThread.IsAlive;
        }
        
        private void StartResponseTimeout()
        {
            StopResponseTimeoutIfRunning();

            if (ResponseTimeoutTimer == null)
            {
                ResponseTimeoutTimer = new System.Timers.Timer(USB_RESPONSE_TIMEOUT_MS);
                ResponseTimeoutTimer.Elapsed += OnResponseTimeout;
                ResponseTimeoutTimer.AutoReset = false;
                ResponseTimeoutTimer.Enabled = true;
                ResponseTimeoutTimer.Start();
                IsRequestPending = true;
            } 
        }

        private void StopResponseTimeoutIfRunning()
        {
            if (ResponseTimeoutTimer != null && ResponseTimeoutTimer.Enabled)
            {
                ResponseTimeoutTimer.Stop();
                ResponseTimeoutTimer = null;
            }
            IsRequestPending = false;
        }

        private void OnResponseTimeout(object source, ElapsedEventArgs e)
        {
            StopResponseTimeoutIfRunning();
            IsRequestPending = false;
        }

        public void SendControl(bool hostready, bool offHook = false)
        {
            if (IsRequestPending)
            {
                Console.WriteLine("THERE IS A REQUEST PENDING! Waiting for it complete");
                WaitForResponse();
            }

            StartResponseTimeout();
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
                        dataItem.WriteRaw(buf, bitOffset, 0, Convert.ToUInt32(offHook));
                        break;
                    default:
                        break;
                }
            });
            stream.Write(buffer);
        }
    }
}
