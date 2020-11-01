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

        // Device that the DTMF helper should send it's sound output into.
        public int DtmfDeviceNumber { get; private set; } = -1;

        // Inbound audio signals (1) Microphone, (2) From Phone Line / 3rd party
        private WaveInEvent FromMic;
        private WaveInEvent FromPhoneLine;

        // Outbound audio signals (1) Local Speakers, (2) To Phone Line / 3rd party
        private WaveOutEvent ToSpeaker;
        private WaveOutEvent ToPhoneLine;

        private BufferedWaveProvider ToPhoneLineBuffer;
        private BufferedWaveProvider ToSpeakerBuffer;

        public MMDevice ToPhoneLineDevice { get; private set; }
        public MMDevice FromPhoneLineDevice { get; private set; }

        /**
         * NAudio MMDevice for the Microphone that's being used by this service.
         * Using this reference, you can use other NAudio APIs to adjust this device's gain or settings.
         */
        // public MMDevice MicDevice { get; private set; }

        /**
         * NAudio MMDevice for the Speaker that's being used by this service.
         * Using this reference, you can use other NAudio APIs to adjust this device's volume or settings.
         */
        // public MMDevice SpeakerDevice { get; private set; }

        public AudioRouter(String rxName = RX_NAME, String txName = TX_NAME)
        {
            RxName = rxName;
            TxName = txName;
        }

        public void Start()
        {
            // This method is not re-entrant, just bail if we're already running.
            if (IsActive)
            {
                return;
            }

            // Get each of the 4 audio devices by name and data flow.
            FromPhoneLineDevice = GetDeviceWithProductName(RxName, DataFlow.Capture);
            ToPhoneLineDevice = GetDeviceWithProductName(TxName, DataFlow.Render);

            // MicDevice = GetCommunicationDeviceFor(DataFlow.Capture);
            // SpeakerDevice = GetCommunicationDeviceFor(DataFlow.Render);

            // EnsureNotNull(ToPhoneLineDevice, FromPhoneLineDevice, SpeakerDevice);

            var fromPhoneLineIndex = WaveInDeviceToIndex(FromPhoneLineDevice);
            var toPhoneLineIndex = MMDeviceToIndex(ToPhoneLineDevice, DataFlow.Render);
            var micIndex = -1; //  Use Default Input Device (Control Panel Green Check)
            var speakerIndex = -1; //  Use Default Output Device (Control Panel Green Check)
            DtmfDeviceNumber = toPhoneLineIndex;

            Console.WriteLine("FromPhoneLine index: " + fromPhoneLineIndex + " Name: " + FromPhoneLineDevice.FriendlyName);
            Console.WriteLine("ToPhoneLine index: " + toPhoneLineIndex + " Name: " + ToPhoneLineDevice.FriendlyName);

            // Configure the DTMF signal device number.

            var waveFormat = new WaveFormat();

            // Connect the local Mic input to the Phone TX line
            FromMic = new WaveInEvent()
            {
                NumberOfBuffers = 2,
                DeviceNumber = micIndex
            };
            FromMic.WaveFormat = waveFormat;
            FromMic.DataAvailable += onMicDataAvailable;

            ToPhoneLine = new WaveOutEvent() {
                DeviceNumber = toPhoneLineIndex,
            };
            ToPhoneLineBuffer = new BufferedWaveProvider(waveFormat);
            ToPhoneLine.Init(ToPhoneLineBuffer);
            ToPhoneLine.Play();
            FromMic.StartRecording();

            // ToSpeaker = new DirectSoundOut();
            // Connect the Phone RX line to the local Speakers
            FromPhoneLine = new WaveInEvent()
            {
                DeviceNumber = fromPhoneLineIndex,
            };
            FromPhoneLine.WaveFormat = waveFormat;
            FromPhoneLine.DataAvailable += onPhoneDataAvailable;

            ToSpeaker = new WaveOutEvent() {
                DesiredLatency = 100,
                NumberOfBuffers = 2,
                DeviceNumber = speakerIndex,
            };
            ToSpeakerBuffer = new BufferedWaveProvider(waveFormat);
            ToSpeaker.Init(ToSpeakerBuffer);
            ToSpeaker.Play();
            FromPhoneLine.StartRecording();

            IsActive = true;
        }

        public void Stop()
        {
            if (IsActive)
            {
                Console.WriteLine("AudioRouter.Stop()");
                FromMic?.StopRecording();
                FromPhoneLine?.StopRecording();
                ToPhoneLine?.Stop();
                ToSpeaker?.Stop();
                FromMic = null;
                ToPhoneLine = null;
                IsActive = false;
            }
        }

        private void EnsureNotNull(MMDevice toPhoneLineDevice, MMDevice fromPhoneLineDevice, MMDevice micDevice, MMDevice speakerDevice)
        {
            if (toPhoneLineDevice == null)
            {
                throw new Exception("Cannot find requested rx device at: " + RxName);
            }
            if (fromPhoneLineDevice == null)
            {
                throw new Exception("Cannot find requested tx device at: " + TxName);
            }
            if (micDevice == null)
            {
                throw new Exception("Cannot find Default Communication input device");
            }
            if (speakerDevice == null)
            {
                throw new Exception("Cannot find Default Communication output device");
            }
        }

        /**
         * Get the index for the provided MMDevice as it relates to available
         * WaveIn devices.
         */
        private int WaveInDeviceToIndex(MMDevice device)
        {
            for (var i = -1; i < WaveInEvent.DeviceCount; i++)
            {
                var capabilities = WaveInEvent.GetCapabilities(i);
                if (capabilities.ProductName == device.FriendlyName)
                {
                    return i;
                }
            }
            return -1;
        }

        private int MMDeviceToIndex(MMDevice device, DataFlow direction = DataFlow.All, DeviceState state = DeviceState.Active)
        {
            var enumerator = new MMDeviceEnumerator();
            var index = 0;
            foreach (var other in enumerator.EnumerateAudioEndPoints(direction, state))
            {
                if (other.FriendlyName == device.FriendlyName)
                {
                    Console.WriteLine("Matched: " + other.FriendlyName);
                    return index;
                }
                index++;
            } 
            return -1;
        }

        private void onMicDataAvailable(object sender, WaveInEventArgs e)
        {
            byte[] buffer = new byte[e.BytesRecorded];
            Buffer.BlockCopy(e.Buffer, 0, buffer, 0, e.BytesRecorded);
            ToPhoneLineBuffer.AddSamples(buffer, 0, e.BytesRecorded);
        }

        private void onPhoneDataAvailable(object sender, WaveInEventArgs e)
        {
            // Console.WriteLine("PHONE BYTES RECEIVED WITH: " + e.BytesRecorded);
            byte[] buffer = new byte[e.BytesRecorded];
            Buffer.BlockCopy(e.Buffer, 0, buffer, 0, e.BytesRecorded);
            ToSpeakerBuffer.AddSamples(buffer, 0, e.BytesRecorded);
        }

        private MMDevice GetCommunicationDeviceFor(DataFlow direction)
        {
            using (var enumerator = new MMDeviceEnumerator())
            {
                var commDevice = enumerator.GetDefaultAudioEndpoint(direction, Role.Communications);
                var devices = enumerator.EnumerateAudioEndPoints(direction, DeviceState.Active);

                foreach (var device in devices)
                {
                    if (device.FriendlyName == commDevice.FriendlyName)
                    {
                        Console.WriteLine("FOUND COMM DEVICE WITH: " + device.FriendlyName);
                        return commDevice;
                    }
                }

                return null;
            }
        }

        private MMDevice GetDeviceWithProductName(String name, DataFlow direction)
        {
            using (var enumerator = new MMDeviceEnumerator())
            {
                // Get the list of audio devices.
                var devices = enumerator.EnumerateAudioEndPoints(direction, DeviceState.Active);
                foreach (var device in devices)
                {
                    if (device.FriendlyName.Contains(name))
                    {
                        Console.WriteLine("GetDeviceWithProductName friendlyName: " + device.FriendlyName + " direction: " + direction);
                        return device;
                    }
                }
            }
            return null;
        }

        public void Dispose()
        {
            Stop();
            FromMic?.Dispose();
            FromPhoneLine?.Dispose();
            ToPhoneLine?.Dispose();
            ToSpeaker?.Dispose();
            // MicDevice?.Dispose();
            // SpeakerDevice?.Dispose();
            IsActive = false;
        }
    }
}
