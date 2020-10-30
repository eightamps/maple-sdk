using NAudio.CoreAudioApi;
using NAudio.Wave;
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

        public bool IsActive { get; private set; } = false;

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

        public int DtmfDeviceNumber
        {
            get
            {
                return 0;
            }
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
            if (IsActive)
            {
                return;
            }

            IsActive = true;

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
            IsActive = false;
        }

        public void Dispose()
        {
            Stop();
            MicSignal?.Dispose();
            PhoneSignal?.Dispose();
            PhoneOutput?.Dispose();
            SpeakerOutput?.Dispose();
            IsActive = false;
        }
    }
}
