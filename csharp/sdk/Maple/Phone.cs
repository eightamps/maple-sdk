using HidSharp.Reports.Input;
using HidSharp.Reports;
using HidSharp;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using System.Timers;
using System;

namespace Maple
{
    public enum PhonyState
    {
        NotReady = 0,
        FirstState = NotReady,
        Ready,
        OffHook,
        Ringing,
        LineNotFound,
        LineInUse,
        HostNotFound,
    }

    enum RingState
    {
        Waiting,
        Ringing,
    }

    public class Phone : IDisposable
    {
        private const long RING_DURATION_MS = 1800;
        private const int USB_TIMEOUT_MS = 50;
        private const int TIMEOUT_MS = 500;
        private const int STITCHER_TIMEOUT_MS = 3000;
        private const int STITCHER_DELAY_MS = 1000;
        private const long OFF_HOOK_TIMEOUT_MS = 1000;

        private readonly AudioStitcher stitcher;
        private readonly Dtmf dtmf;
        private Thread dtmfThread;
        private bool isRequestPending;
        private bool _OffHook;

        private HidDevice hiddev { get { return stream.Device; } }
        private readonly HidStream stream;
        private RingState ringState = RingState.Waiting;
        private System.Timers.Timer ringTimer;

        private readonly Report verReport;
        private readonly Report txReport;
        private readonly HidDeviceInputReceiver inputReceiver;
        private readonly DeviceItemInputParser inputParser;

        public event Action<Phone, bool> RingingChanged = delegate { };
        public event Action<Phone, bool> LineIsAvailableChanged = delegate { };
        public event Action<Phone, bool> OffHookChanged = delegate { };
        public event Action<Phone, bool> PolarityChanged = delegate { };
        public event Action<Phone> Disconnected = delegate { };
        public static readonly String PhoneCapture = "ASI Telephone";
        public static readonly String PhoneRender = "ASI Telephone";

        public PhonyState PhoneState { get; private set; }
        public Version SoftwareVersion { get; private set; }
        public Version HardwareVersion { get { return hiddev.ReleaseNumber; } }
        public bool CommunicationFailure { get; private set; }

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
                    LineIsAvailableChanged(this, lineIsAvailable);
                }
            }
        }
        public bool OffHook
        {
            get
            {
                return _OffHook;
            }
            private set
            {
                if (value != _OffHook)
                {
                    _OffHook = value;
                    SyncStitcherToHookState();
                    OffHookChanged(this, _OffHook);
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
            this.inputReceiver.Received -= InputHandler;

            // Only attempt to notify the device if we haven't already
            // had a communication failure.
            if (this.CommunicationFailure == false)
            {
                SendControl(false);
            }

            if (ringTimer != null)
            {
                ClearRingTimer();
            }

            if (DtmfDialIsInProgress())
            {
                dtmfThread.Abort();
            }

            stitcher.Dispose();
            stream.Dispose();
            this._OffHook = false;
        }

        private void InputHandler(object sender, EventArgs e)
        {
            var inputReportBuffer = new byte[hiddev.GetMaxInputReportLength()];
            Report report;
            var sw = Stopwatch.StartNew();
            while (inputReceiver.TryRead(inputReportBuffer, 0, out report) &&
                sw.ElapsedMilliseconds < TIMEOUT_MS)
            {
                // Parse the report if possible.
                // This will return false if (for example) the report applies to a different DeviceItem.
                if (inputParser.TryParseReport(inputReportBuffer, 0, report))
                {
                    while (inputParser.HasChanged)
                    {
                        int changedIndex = inputParser.GetNextChangedIndex();
                        // var previousDataValue = inputParser.GetPreviousValue(changedIndex);
                        var dataValue = inputParser.GetValue(changedIndex);

                        var usages = dataValue.Usages.FirstOrDefault();
                        switch (usages)
                        {
                            case (uint)HidUsage.Telephony.HookState:
                                OffHook = Convert.ToBoolean(dataValue.GetLogicalValue());
                                break;
                            case (uint)HidUsage.Telephony.RingState:
                                IsRinging = Convert.ToBoolean(dataValue.GetLogicalValue());
                                break;
                            case (uint)HidUsage.Telephony.LineIsAvailable:
                                LineIsAvailable = Convert.ToBoolean(dataValue.GetLogicalValue());
                                break;
                            case (uint)HidUsage.EightAmps.PhonyState:
                                PhoneState = (PhonyState)dataValue.GetLogicalValue();
                                break;
                            default:
                                Console.WriteLine("Unhandled Phony Input Event:" + usages);
                                break;
                        }
                    }
                }
            }
            sw.Stop();
            isRequestPending = false;
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
            if (LineIsAvailable)
            {
                SendControl(true, true);
                WaitForOffHook();
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
                dtmfThread.Abort();
            }

            SendControl(true, false);
            WaitForResponse();
            return !OffHook;
        }

        private void WaitForOffHook()
        {
            if (!OffHook)
            {
                var sw = Stopwatch.StartNew();
                while (!_OffHook && sw.ElapsedMilliseconds < OFF_HOOK_TIMEOUT_MS)
                {
                    // Wait for firmware confirmation that the line is Off Hook,
                    // or timeout expired.
                    Thread.Sleep(TimeSpan.FromMilliseconds(100));
                }
                sw.Stop();
            }
        }

        private void WaitForStitcher()
        {
            // Wait for the audio router to get everything wired up.
            var sw = Stopwatch.StartNew();
            while (!stitcher.IsActive && sw.ElapsedMilliseconds < STITCHER_TIMEOUT_MS)
            {
                Thread.Sleep(TimeSpan.FromMilliseconds(5));
            }
            sw.Stop();

            // Wait another few seconds for everything to be ready.
            Thread.Sleep(TimeSpan.FromMilliseconds(STITCHER_DELAY_MS));
        }

        private void WaitForResponse()
        {
            if (isRequestPending)
            {
                var sw = Stopwatch.StartNew();
                while (isRequestPending && sw.ElapsedMilliseconds < USB_TIMEOUT_MS)
                {
                    Thread.Sleep(TimeSpan.FromMilliseconds(10));
                }
                sw.Stop();

                // We've either timed out our the request status was updated externally.
                isRequestPending = false;
            }
        }

        public bool Dial(String input)
        {
            if (!LineIsAvailable)
            {
                return false;
            }

            // Ensure we're not already waiting for a response.
            WaitForResponse();

            if (!OffHook)
            {
                TakeOffHook();
            }

            WaitForStitcher();

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
                dtmfThread.Join();
            }

            dtmfThread = new Thread(() =>
            {
                // Send the DTMF codes through the open line.
                dtmf.GenerateDtmfTones(input, stitcher);
            });
            dtmfThread.Start();
        }
        private bool DtmfDialIsInProgress()
        {
            return dtmfThread != null && dtmfThread.IsAlive;
        }

        public void SendControl(bool hostready, bool offHook = false)
        {
            if (isRequestPending)
            {
                WaitForResponse();
            }

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
                        dataItem.WriteRaw(buf, bitOffset, 0, Convert.ToUInt32(offHook));
                        break;
                    default:
                        break;
                }
            });

            try
            {
                isRequestPending = true;
                stream.Write(buffer);
            }
            catch (IOException)
            {
                this.CommunicationFailure = true;
                Disconnected(this);
            }
        }
    }
}
