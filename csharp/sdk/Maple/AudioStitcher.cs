using NAudio.CoreAudioApi;
using NAudio.Wave;
using System;

namespace Maple
{
    public class AudioStitcher : IDisposable
    {
        public const int DEFAULT_LATENCY = 40;
        public const bool DEFAULT_SYNC = true;

        public const int DEFAULT_OUT_SAMPLE_RATE = 44100;
        public const int DEFAULT_OUT_CH = 1;

        public const int DEFAULT_IN_SAMPLE_RATE = 48000;
        public const int DEFAULT_IN_CH = 1;
        public const AudioClientShareMode DEFAULT_SHARE = AudioClientShareMode.Shared;

        public String RxName { get; private set; }
        public String TxName { get; private set; }

        public bool IsActive { get; private set; }

        public MMDevice ToPhoneLineDevice { get; private set; }
        public MMDevice FromPhoneLineDevice { get; private set; }

        public MMDevice ToSpeakerDevice { get; private set; }
        public MMDevice FromMicDevice { get; private set; }

        public WasapiCapture FromMicChannel { get; private set; }
        public WasapiOut ToPhoneLineChannel { get; private set; }

        public WasapiCapture FromPhoneLineChannel { get; private set; }
        public WasapiOut ToSpeakerChannel { get; private set; }

        public BufferedWaveProvider ToSpeakerBuffer { get; private set; }
        public BufferedWaveProvider FromMicBuffer { get; private set; }
        public MixingWaveProvider32 ToPhoneLineMixer { get; private set; }

        public AudioStitcher(String rxName, String txName)
        {
            RxName = rxName;
            TxName = txName;
        }

        public void Start()
        {
            if (IsActive)
            {
                return;
            }
            StartWasapi();
        }

        private void LogWaveFormat(String label, WaveFormat waveFormat)
        {
            Console.WriteLine(label + " ENC: " + waveFormat.Encoding + " SR: " + waveFormat.SampleRate + " CH: " + waveFormat.Channels);
        }

        public void StartWasapi()
        {
            Console.WriteLine("----------------------------");
            Console.WriteLine("Wasapi AudioStitcher.Start()");
            IsActive = true;

            // Get each of the 4 audio devices by name and data flow.
            FromPhoneLineDevice = GetMMDeviceByName(RxName, DataFlow.Capture);
            ToPhoneLineDevice = GetMMDeviceByName(TxName, DataFlow.Render);

            ToSpeakerDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Render, Role.Communications);
            FromMicDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Capture, Role.Communications);

            // Configure the Line to Speaker connection.
            FromPhoneLineChannel = new WasapiCapture(FromPhoneLineDevice, DEFAULT_SYNC, DEFAULT_LATENCY);
            FromPhoneLineChannel.DataAvailable += FromPhoneLineDataAvailable;

            ToSpeakerChannel = new WasapiOut(ToSpeakerDevice, DEFAULT_SHARE, DEFAULT_SYNC, DEFAULT_LATENCY);
            ToSpeakerBuffer = new BufferedWaveProvider(FromPhoneLineChannel.WaveFormat);

            // Configure the Mic to Line connection.
            FromMicChannel = new WasapiCapture(FromMicDevice, DEFAULT_SYNC, DEFAULT_LATENCY);
            FromMicChannel.DataAvailable += FromMicDataAvailable;

            // LogWaveFormat("FromPhoneLine:    ", FromPhoneLineChannel.WaveFormat);
            // LogWaveFormat("FromMic:   ", FromMicChannel.WaveFormat);
            // LogWaveFormat("ToSpeaker: ", ToSpeakerChannel.OutputWaveFormat);
            // LogWaveFormat("ToPhoneLine:      ", ToPhoneLineChannel.OutputWaveFormat);

            ToPhoneLineChannel = new WasapiOut(ToPhoneLineDevice, DEFAULT_SHARE, DEFAULT_SYNC, DEFAULT_LATENCY);
            FromMicBuffer = new BufferedWaveProvider(FromMicChannel.WaveFormat);

            ToPhoneLineMixer = new MixingWaveProvider32();
            ToPhoneLineMixer.AddInputStream(FromMicBuffer);

            ToSpeakerChannel.Init(ToSpeakerBuffer);
            ToPhoneLineChannel.Init(ToPhoneLineMixer);

            // Start doing work now.
            FromPhoneLineChannel.StartRecording();
            FromMicChannel.StartRecording();
            ToSpeakerChannel.Play();
            ToPhoneLineChannel.Play();
        }

        public void Stop()
        {
            if (!IsActive)
            {
                return;
            }

            // Dispose of everything.
            ToSpeakerChannel?.Dispose();
            FromPhoneLineChannel?.Dispose();
            FromMicChannel?.Dispose();
            ToPhoneLineChannel?.Dispose();
            IsActive = false;
        }

        private int phoneLineDeviceNumber = -2;
        public int ToPhoneLineDeviceNumber()
        {
            if (phoneLineDeviceNumber == -2)
            {
                var enumerator = new MMDeviceEnumerator().EnumerateAudioEndPoints(DataFlow.Render, DeviceState.Active);
                var index = 0;
                var foundIndex = -1;
                foreach (var device in enumerator)
                {
                    Console.WriteLine("CHECKING index: " + index + " for: " + device.FriendlyName);
                    if (device.FriendlyName == ToPhoneLineDevice.FriendlyName)
                    {
                        foundIndex = index;
                    }
                    index++;
                }

                phoneLineDeviceNumber = foundIndex;
            }

            return phoneLineDeviceNumber;
        }

        private void FromPhoneLineDataAvailable(object sender, WaveInEventArgs e)
        {
            // Console.WriteLine("FROM PHONE LINE DATA AVAIL!");
            byte[] buffer = new byte[e.BytesRecorded];
            Buffer.BlockCopy(e.Buffer, 0, buffer, 0, e.BytesRecorded);
            ToSpeakerBuffer.AddSamples(buffer, 0, e.BytesRecorded);
        }

        private void FromMicDataAvailable(object sender, WaveInEventArgs e)
        {
            // Console.WriteLine("FROM PHONE LINE DATA AVAIL!");
            byte[] buffer = new byte[e.BytesRecorded];
            Buffer.BlockCopy(e.Buffer, 0, buffer, 0, e.BytesRecorded);
            FromMicBuffer.AddSamples(buffer, 0, e.BytesRecorded);
        }

        private int GetDeviceIndexFor(String name)
        {
            var enumerator = new MMDeviceEnumerator();
            var index = 0;
            foreach (var device in enumerator.EnumerateAudioEndPoints(DataFlow.Render, DeviceState.Active))
            {
                if (device.FriendlyName.Contains(name))
                {
                    // Console.WriteLine("Found requested device at index: " + index);
                    return index;
                }
                index++;
            }
            return -1;
        }

        private DirectSoundDeviceInfo GetDsoDeviceByName(String name = "")
        {
            foreach (var device in DirectSoundOut.Devices)
            {
                if (name == "" || device.Description.Contains(name)) 
                {
                    // Console.WriteLine("FOUND MATCH:" + name);
                    return device;
                }
            }
            return null;
        }

        private MMDevice GetMMDeviceByName(String name, DataFlow direction)
        {
            using (var enumerator = new MMDeviceEnumerator())
            {
                // Get the list of audio devices.
                var devices = enumerator.EnumerateAudioEndPoints(direction, DeviceState.Active);
                foreach (var device in devices)
                {
                    if (device.FriendlyName.Contains(name))
                    {
                        // Console.WriteLine("GetDeviceWithProductName friendlyName: " + device.FriendlyName + " direction: " + direction);
                        return device;
                    }
                }
            }
            return null;
        }

        public void Dispose()
        {
            if (IsActive)
            {
                Stop();
            }
        }
    }
}
