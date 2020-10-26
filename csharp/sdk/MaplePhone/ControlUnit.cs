using HidSharp;
using HidSharp.Reports;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace Maple
{
    public class ControlUnit : IDisposable
    {
        public Version SoftwareVersion { get; private set; }
        public Version HardwareVersion { get { return hiddev.ReleaseNumber; } }

        protected ControlUnit(HidStream hidStream)
        {
            this.stream = hidStream;
            this.reports = new Dictionary<HidUsage.EightAmps, Report>();

            var reportDescriptor = hiddev.GetReportDescriptor();
            var deviceItem = reportDescriptor.DeviceItems.First();
            var freports = deviceItem.FeatureReports;
            var oreports = deviceItem.OutputReports;
            var ireports = deviceItem.InputReports;

            if (false)
            {
                this.verReport = freports.First(r => r.GetAllUsages().Contains((uint)HidUsage.GenericDevice.SoftwareVersion));
                reports.Add(HidUsage.EightAmps.HaGetCapabilitiesReport, freports.First(r => r.GetAllUsages().Contains((uint)HidUsage.EightAmps.HaGetCapabilitiesReport)));
                reports.Add(HidUsage.EightAmps.HaGetTerminalCapabilitiesReport, freports.First(r => r.GetAllUsages().Contains((uint)HidUsage.EightAmps.HaGetTerminalCapabilitiesReport)));
                reports.Add(HidUsage.EightAmps.HaSetTerminalStateReport, oreports.First(r => r.GetAllUsages().Contains((uint)HidUsage.EightAmps.HaSetTerminalStateReport)));
            }
            else
            {
                this.verReport = freports.First(r => r.GetAllUsages().Contains((uint)HidUsage.GenericDevice.Major));
                // hardcoding as fallback
                // could also consider sw version
                reports.Add(HidUsage.EightAmps.HaGetCapabilitiesReport, freports.First(r => r.ReportID == 1));
                reports.Add(HidUsage.EightAmps.HaGetTerminalCapabilitiesReport, freports.First(r => r.ReportID == 2));
                reports.Add(HidUsage.EightAmps.HaSetTerminalStateReport, oreports.First(r => r.ReportID == 1));
            }

            // read SW version
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

            // get global capabilities
            var getCapReport = reports[HidUsage.EightAmps.HaGetCapabilitiesReport];
            buffer = getCapReport.CreateBuffer();
            stream.GetFeature(buffer);
            getCapReport.Read(buffer, 0, (dataValue) =>
            {
                var usage = (HidUsage.EightAmps)dataValue.Usages.FirstOrDefault();
                uint val = (uint)dataValue.GetLogicalValue();
                switch (usage)
                {
                    case HidUsage.EightAmps.HaInTerminalsCount:
                        inTerminals = new uint[val];
                        Array.Clear(inTerminals, 0, inTerminals.Length);
                        break;
                    case HidUsage.EightAmps.HaOutTerminalsCount:
                        outTerminals = new uint[val];
                        Array.Clear(outTerminals, 0, outTerminals.Length);
                        break;
                    default:
                        break;
                }
            });

            // get terminal capabilities for each terminal
            var getTermCapReport = reports[HidUsage.EightAmps.HaGetTerminalCapabilitiesReport];
            buffer = getTermCapReport.CreateBuffer();
            for (uint i = 0; i < (inTerminals.Length + outTerminals.Length); i++)
            {
                stream.GetFeature(buffer);
                int termId = -1;
                int contacts = -1;
                getTermCapReport.Read(buffer, 0, (dataValue) =>
                {
                    var usage = (HidUsage.EightAmps)dataValue.Usages.FirstOrDefault();
                    int val = dataValue.GetLogicalValue();
                    switch (usage)
                    {
                        case HidUsage.EightAmps.HaTerminalId:
                            termId = val;
                            break;
                        case HidUsage.EightAmps.HaTerminalContactsCount:
                            contacts = val;
                            break;
                        default:
                            break;
                    }
                });
                if ((0 <= termId) && (termId < (inTerminals.Length + outTerminals.Length)) && (contacts > 0))
                {
                    if (termId < inTerminals.Length)
                    {
                        inTerminals[termId] = (uint)contacts;
                    }
                    else
                    {
                        outTerminals[termId - inTerminals.Length] = (uint)contacts;
                    }
                }
            }
        }
        private HidDevice hiddev { get { return stream.Device; } }
        private HidStream stream;
        private Dictionary<HidUsage.EightAmps, Report> reports;
        private Report verReport;
        private uint[] inTerminals = new uint[0];
        private uint[] outTerminals = new uint[0];

        public void Dispose()
        {
            stream.Dispose();
        }

        public void SetTerminal(uint outTerminalId, List<ValueTuple<uint, bool>> contactsToSet)
        {
            if (outTerminalId >= outTerminals.Length)
            {
                throw new ArgumentOutOfRangeException("outTerminalId", outTerminalId, "Must be a valid selector of an output terminal");
            }

            if (contactsToSet.Any((t) => t.Item1 >= outTerminals[outTerminalId]))
            {
                throw new ArgumentOutOfRangeException("contactsToSet", contactsToSet, "Must have valid contact indexes only");
            }

            uint terminalId = outTerminalId + (uint)inTerminals.Length;
            var report = reports[HidUsage.EightAmps.HaSetTerminalStateReport];
            var buffer = report.CreateBuffer();

            report.Write(buffer, 0, (buf, bitOffset, dataItem, indexOfDataItem) =>
            {
                var usage = (HidUsage.EightAmps)dataItem.Usages.GetAllValues().FirstOrDefault();
                switch (usage)
                {
                    case HidUsage.EightAmps.HaTerminalId:
                        dataItem.WriteRaw(buf, bitOffset, 0, terminalId);
                        break;
                    case HidUsage.EightAmps.HaTerminalContactValue:
                        foreach (var contact in contactsToSet)
                        {
                            dataItem.WriteRaw(buf, bitOffset, (int)contact.Item1, (uint)(contact.Item2 ? 1 : 3));
                        }
                        break;
                    default:
                        break;
                }
            });
            stream.Write(buffer);
        }

        public static bool TryOpen(HidDevice hiddev, out ControlUnit CtrlUnit)
        {
            CtrlUnit = null;
            if (!hiddev.IsMapleControlDevice())
                return false;
            if (hiddev.GetTopLevelUsage() != (uint)HidUsage.EightAmps.Envy)
                return false;

            try
            {
                HidStream hidStream;
                if (!hiddev.TryOpen(out hidStream))
                    return false;
                hidStream.ReadTimeout = Timeout.Infinite;
                CtrlUnit = new ControlUnit(hidStream);
            }
            catch (Exception)
            {
                return false;
            }
            return true;
        }
        public static ControlUnit First()
        {
            ControlUnit r = null;
            var device = DeviceList.Local.GetHidDevices().First(d => ControlUnit.TryOpen(d, out r));
            return r;
        }
    }
}
