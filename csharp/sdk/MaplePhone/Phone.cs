using HidSharp;
using HidSharp.Reports;
using HidSharp.Reports.Input;
using System;
using System.Linq;
using System.Threading;

namespace MaplePhone
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
        private bool loopState = false;

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
            this.inputReceiver.Received += InputReceiver_Received;
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
            catch (Exception)
            {
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
            this.inputReceiver.Received -= InputReceiver_Received;
            SendControl(false);
            router.Dispose();
            stream.Dispose();
        }

        private void InputReceiver_Received(object sender, EventArgs e)
        {
            var inputReportBuffer = new byte[hiddev.GetMaxInputReportLength()];
            Report report;
            while (inputReceiver.TryRead(inputReportBuffer, 0, out report))
            {
                // Parse the report if possible.
                // This will return false if (for example) the report applies to a different DeviceItem.
                if (inputParser.TryParseReport(inputReportBuffer, 0, report))
                {
                    bool ringingChanged = false;
                    bool isRinging = false;

                    while (inputParser.HasChanged)
                    {
                        int changedIndex = inputParser.GetNextChangedIndex();
                        var previousDataValue = inputParser.GetPreviousValue(changedIndex);
                        var dataValue = inputParser.GetValue(changedIndex);

                        switch ((HidUsage.Telephony)dataValue.Usages.FirstOrDefault())
                        {
                            case HidUsage.Telephony.HookSwitch:
                                RemoteOffHook(this, Convert.ToBoolean(dataValue.GetLogicalValue()));
                                break;
                            case HidUsage.Telephony.AlternateFunction:
                                Polarity(this, Convert.ToBoolean(dataValue.GetLogicalValue()));
                                break;
                            case HidUsage.Telephony.RingEnable:
                                ringingChanged = true;
                                isRinging |= Convert.ToBoolean(dataValue.GetLogicalValue());
                                Console.WriteLine("RingEnable " + dataValue.DataIndex);
                                break;
                            case HidUsage.Telephony.HostControl:
                                loopState = Convert.ToBoolean(dataValue.GetLogicalValue());
                                LoopPresence(this, loopState);
                                break;
                            default:
                                break;
                        }
                    }

                    if (ringingChanged)
                    {
                        RingingSignal(this, isRinging);
                    }
                }
            }
        }

        public void SetOffHook(bool offhook)
        {
            // Never take the phone OFF_HOOK unless LOOP detect indicates a valid line is attached
            if (!offhook || loopState)
            {
                SendControl(true, offhook);
            }
            SendControl(true, offhook);
        }

        public void Dial(String phoneNumbers)
        {
            if (loopState || false) 
            {
                // If we're not already off-hook, go off-hook and then dial
                SetOffHook(true);
                router.Start();
                Thread.Sleep(TimeSpan.FromSeconds(2));
            }

            // Send the DTMF codes through the open line.
            router.GenerateTones(phoneNumbers);
        }

        public void SendControl(bool hostready, bool offhook = false)
        {
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
                        dataItem.WriteRaw(buf, bitOffset, 0, Convert.ToUInt32(offhook));
                        break;
                    default:
                        break;
                }
            });
            stream.Write(buffer);
        }
    }
}
