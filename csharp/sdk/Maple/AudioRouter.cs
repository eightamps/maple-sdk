using NAudio.CoreAudioApi;
using NAudio.Wave.SampleProviders;
using NAudio.Wave;
using System.Collections.Generic;
using System.Threading;
using System;

namespace Maple
{
    class AudioRouter : IDisposable
    {
        const String RX_NAME = "Telephone Audio"; // First string, stuck in dev cache.
        const String TX_NAME = "Telephone Audio"; // First string, stuck in dev cache.
        // const String RX_NAME = "ASI Telephone"; // Actual configuration strings
        // const String TX_NAME = "ASI Telephone"; // Actual configuration strings
        // const String RX_NAME = "4- USB Audio Device"; // Unconfigured HS100B for dev
        // const String TX_NAME = "4- USB Audio Device"; // Unconfigured HS100B for dev

        public String RxName { get; set; }
        public String TxName { get; set; }

        private bool _IsRunning = false;
        public bool IsRunning { get { return _IsRunning;  } }

        private Dictionary<char, Tuple<int, int>> DtmfLookup;

        private WaveInEvent MicSignal;
        private WaveInEvent PhoneSignal;
        private WaveOutEvent PhoneOutput;
        private WaveOutEvent SpeakerOutput;
        private BufferedWaveProvider PhoneOutputBuffer;
        private BufferedWaveProvider SpeakerOutputBuffer;

        public AudioRouter(String rxName = RX_NAME, String txName = TX_NAME)
        {
            RxName = rxName;
            TxName = txName;
        }

        private void onMicDataAvailable(object sender, WaveInEventArgs e)
        {
            // Console.WriteLine("BYTES RECEIVED WITH: " + e.BytesRecorded);
            byte[] buffer = new byte[e.BytesRecorded];
            Buffer.BlockCopy(e.Buffer, 0, buffer, 0, e.BytesRecorded);
            PhoneOutputBuffer.AddSamples(buffer, 0, e.BytesRecorded);
        }

        private void onPhoneDataAvailable(object sender, WaveInEventArgs e)
        {
            // Console.WriteLine("PHONE BYTES RECEIVED WITH: " + e.BytesRecorded);
            byte[] buffer = new byte[e.BytesRecorded];
            Buffer.BlockCopy(e.Buffer, 0, buffer, 0, e.BytesRecorded);
            SpeakerOutputBuffer.AddSamples(buffer, 0, e.BytesRecorded);
        }

        private void GenerateDtmf(TimeSpan duration, int top, int bottom, int deviceNumber=-1)
        {
            Console.WriteLine("GENERATE DTMF WITH deviceNumber: " + deviceNumber);
            var one = new SignalGenerator()
            {
                Gain = 0.2,
                Frequency = top,
                Type = SignalGeneratorType.Sin
            }.Take(duration);

            var two = new SignalGenerator()
            {
                Gain = 0.2,
                Frequency = bottom,
                Type = SignalGeneratorType.Sin
            }.Take(duration);

            using (var wOne = new WaveOutEvent())
            using (var wTwo = new WaveOutEvent())
            {
                if (deviceNumber > -1)
                {
                    wOne.DeviceNumber = deviceNumber;
                    wTwo.DeviceNumber = deviceNumber;
                }

                wOne.Init(one);
                wOne.Play();

                wTwo.Init(two);
                wTwo.Play();
                while (wOne.PlaybackState == PlaybackState.Playing ||
                    wTwo.PlaybackState == PlaybackState.Playing)
                {
                    Thread.Sleep(1);
                }
            }
        }

        private Dictionary<char, Tuple<int, int>> GetLookup()
        {
            if (DtmfLookup == null)
            {
                Dictionary<char, Tuple<int, int>> values = new Dictionary<char, Tuple<int, int>>();
                values.Add('1', Tuple.Create(697, 1209));
                values.Add('2', Tuple.Create(697, 1336));
                values.Add('3', Tuple.Create(697, 1477));
                values.Add('A', Tuple.Create(697, 1633));

                values.Add('4', Tuple.Create(770, 1209));
                values.Add('5', Tuple.Create(770, 1336));
                values.Add('6', Tuple.Create(770, 1477));
                values.Add('B', Tuple.Create(770, 1633));

                values.Add('7', Tuple.Create(852, 1209));
                values.Add('8', Tuple.Create(852, 1336));
                values.Add('9', Tuple.Create(852, 1477));
                values.Add('C', Tuple.Create(852, 1633));

                values.Add('*', Tuple.Create(941, 1209));
                values.Add('0', Tuple.Create(941, 1336));
                values.Add('#', Tuple.Create(941, 1477));
                values.Add('D', Tuple.Create(941, 1633));
                DtmfLookup = values;
            }

            return DtmfLookup;
        }

        private Tuple<int, int>[] StringToDtmf(String value)
        {
            var lookup = GetLookup();
            var len = value.Length;
            Tuple<int, int>[] tones = new Tuple<int, int>[len];

            var i = 0;
            foreach(var entry in value.ToCharArray())
            {
                Console.WriteLine("Dialing: " + entry);
                tones[i] = lookup[entry];
                i++;
            }
            return tones;
        }

        public void GenerateTones(String values)
        {
            if (!IsRunning)
            {
                Start();
            }
            // Get the device index from the PhoneOutput signal.
            var deviceNumber = PhoneOutput.DeviceNumber;
            Console.WriteLine("GENERATE TONES WITH:" + deviceNumber);
            var duration = TimeSpan.FromMilliseconds(100);
            var tones = StringToDtmf(values);
            foreach (var tone in tones)
            {
                GenerateDtmf(duration, tone.Item1, tone.Item2, deviceNumber);
            }
        }

        private Tuple<MMDevice, int> GetCommunicationDeviceFor(DataFlow direction)
        {
            using (var enumerator = new MMDeviceEnumerator())
            {
                var commDevice = enumerator.GetDefaultAudioEndpoint(direction, Role.Communications);
                var devices = enumerator.EnumerateAudioEndPoints(direction, DeviceState.Active);

                var commDeviceIndex = 0;
                foreach(var device in devices) 
                {
                    if (device.FriendlyName == commDevice.FriendlyName)
                    {
                        break;
                    }
                    commDeviceIndex++;
                }

                return Tuple.Create(commDevice, commDeviceIndex);
            }
        }

        private Tuple<MMDevice, int> GetDeviceWithProductName(String name, DataFlow direction=DataFlow.Render)
        {
            using (var enumerator = new MMDeviceEnumerator())
            {
                // Get the list of audio devices.
                var devices = enumerator.EnumerateAudioEndPoints(direction, DeviceState.Active);
                for (var i = 0; i < devices.Count; i++) 
                {
                    if (devices[i].FriendlyName.Contains(name))
                    {
                        Console.WriteLine("FOUND MATCH WITH NAME: " + name + "/" + devices[i].FriendlyName);
                        return Tuple.Create(devices[i], i);
                    }
                }
            }
            return null;
        }

        public void Start()
        {
            // This method is not re-entrant, just bail if we're already running.
            if (IsRunning)
            {
                return;
            }

            _IsRunning = true;

            var rxDeviceTuple = GetDeviceWithProductName(RxName, DataFlow.Capture);
            var txDeviceTuple = GetDeviceWithProductName(TxName, DataFlow.Render);

            if (rxDeviceTuple == null)
            {
                throw new Exception("Cannot find requested rx device at: " + RxName);
            }

            if (txDeviceTuple == null)
            {
                throw new Exception("Cannot find requested tx device at: " + TxName);
            }

            var renderTuple = GetCommunicationDeviceFor(DataFlow.Render);
            var captureTuple = GetCommunicationDeviceFor(DataFlow.Capture);

            Console.WriteLine("User Communication Input: " + captureTuple.Item1.DeviceFriendlyName);
            Console.WriteLine("User Communication Output: " + renderTuple.Item1.DeviceFriendlyName);
            Console.WriteLine("Phone RX: " + rxDeviceTuple.Item1.DeviceFriendlyName);
            Console.WriteLine("Phone TX: " + txDeviceTuple.Item1.DeviceFriendlyName);

            var waveFormat = new WaveFormat(22050, 1); // SampleRate & Channels

            // Connect the local Mic input to the Phone Output
            MicSignal = new WaveInEvent() { DeviceNumber = captureTuple.Item2 };
            MicSignal.WaveFormat = waveFormat;
            MicSignal.DataAvailable += onMicDataAvailable;

            PhoneOutput = new WaveOutEvent() { DeviceNumber = rxDeviceTuple.Item2 };
            PhoneOutputBuffer = new BufferedWaveProvider(waveFormat);
            PhoneOutput.Init(PhoneOutputBuffer);
            PhoneOutput.Play();

            // Connect the Phone Signal to the local Speakers
            PhoneSignal = new WaveInEvent() { DeviceNumber = txDeviceTuple.Item2 };
            PhoneSignal.WaveFormat = waveFormat;
            PhoneSignal.DataAvailable += onPhoneDataAvailable;

            SpeakerOutput = new WaveOutEvent() { DeviceNumber = renderTuple.Item2 };
            SpeakerOutputBuffer = new BufferedWaveProvider(waveFormat);
            SpeakerOutput.Init(SpeakerOutputBuffer);
            SpeakerOutput.Play();

            MicSignal.StartRecording();
            PhoneSignal.StartRecording();
        }

        public void Stop()
        {
            MicSignal?.StopRecording();
            PhoneSignal?.StopRecording();
            PhoneOutput?.Stop();
            SpeakerOutput?.Stop();
            MicSignal = null;
            PhoneOutput = null;
            _IsRunning = false;
        }

        public void Dispose()
        {
            Stop();
            MicSignal?.Dispose();
            PhoneSignal?.Dispose();
            PhoneOutput?.Dispose();
            SpeakerOutput?.Dispose();
            _IsRunning = false;
        }
    }
}
