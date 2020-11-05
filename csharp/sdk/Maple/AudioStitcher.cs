using NAudio.CoreAudioApi;
using NAudio.Wave;
using System;
using System.Threading;

namespace Maple
{
    public class AudioStitcher : IDisposable
    {
        public const int DEFAULT_LATENCY = 80;
        public const bool DEFAULT_SYNC = true;

        public const int DEFAULT_OUT_SAMPLE_RATE = 44100;
        public const int DEFAULT_OUT_CH = 1;

        public const int DEFAULT_IN_SAMPLE_RATE = 48000;
        public const int DEFAULT_IN_CH = 1;
        public const AudioClientShareMode DEFAULT_SHARE = AudioClientShareMode.Shared;

        public String RxName { get; private set; }
        public String TxName { get; private set; }

        public bool IsActive { get; private set; }

        public int FromPhoneLineIndex { get; private set; }
        public int ToPhoneLineIndex { get; private set; }
        public int ToSpeakerIndex { get; private set; }
        public int FromMicIndex { get; private set; }
        public MixingWaveProvider32 ToPhoneLineMixer { get; private set; }
        public BufferedWaveProvider FromMicBuffer { get; private set; }
        public Wave16ToFloatProvider FromMicBufferFloat { get; private set; }
        public WaveFormat WaveFormatIeee { get; private set; }
        public WaveFormat ToPhoneLineWaveFormat { get; private set; }
        // public WaveFormat FromPhoneLineWaveFormat { get; private set; }
        // public BufferedWaveProvider FromDtmfBuffer { get; private set; }

        /**
         * =========================================
         * Followng group is used for Wasapi Device
         */
        public MMDevice ToPhoneLineDevice { get; private set; }
        public MMDevice FromPhoneLineDevice { get; private set; }

        public MMDevice ToSpeakerDevice { get; private set; }
        public MMDevice FromMicDevice { get; private set; }

        public WasapiCapture FromMicChannel { get; private set; }
        public WasapiOut ToPhoneLineChannel { get; private set; }

        public WasapiCapture FromPhoneLineChannel { get; private set; }
        public WasapiOut ToSpeakerChannel { get; private set; }

        public BufferedWaveProvider ToSpeakerBuffer { get; private set; }
        public BufferedWaveProvider ToPhoneLineBuffer { get; private set; }


        // =========================================

        /**
         * =========================================
         * Followng group is used for DSO Devices
         */
        public DirectSoundDeviceInfo ToPhoneLineDso { get; private set; }
        public DirectSoundDeviceInfo ToSpeakerDso { get; private set; }

        public DirectSoundOut ToSpeakerDsoChannel { get; private set; }
        // =========================================

        /**
         * =========================================
         * Followng group is used for Wave Devices
         */
        public WaveInEvent FromPhoneLineWave { get; private set; }
        public WaveInEvent FromMicWave { get; private set; }
        public WaveOutEvent ToSpeakerWave { get; private set; }
        public WaveOutEvent ToPhoneLineWave { get; private set; }

        // =========================================

        public int SampleRate
        {
            get { return (int)FromMicChannel?.WaveFormat.SampleRate; }
        }

        public AudioStitcher(String rxName, String txName)
        {
            RxName = rxName;
            TxName = txName;
            ToPhoneLineIndex = GetDeviceIndexFor(TxName);
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

            // var waveFormat = new WaveFormat(44100, 1);
            ToPhoneLineWaveFormat = new WaveFormat(DEFAULT_OUT_SAMPLE_RATE, DEFAULT_OUT_CH);
            // FromPhoneLineWaveFormat = new WaveFormat(DEFAULT_IN_SAMPLE_RATE, DEFAULT_IN_CH);
            // WaveFormatIeee = WaveFormat.CreateIeeeFloatWaveFormat(DEFAULT_SAMPLE_RATE, DEFAULT_OUT_CH);
            // LogWaveFormat("WFIEEE", WaveFormatIeee);

            // Get each of the 4 audio devices by name and data flow.
            FromPhoneLineDevice = GetMMDeviceByName(RxName, DataFlow.Capture);
            ToPhoneLineDevice = GetMMDeviceByName(TxName, DataFlow.Render);

            ToSpeakerDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Render, Role.Communications);
            FromMicDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Capture, Role.Communications);

            // Configure the Line to Speaker connection.
            FromPhoneLineChannel = new WasapiCapture(FromPhoneLineDevice, DEFAULT_SYNC, DEFAULT_LATENCY);
            // FromPhoneLineChannel.WaveFormat = FromPhoneLineWaveFormat;
            // FromPhoneLineChannel.WaveFormat = new WaveFormat(44100, 16, 1);
            FromPhoneLineChannel.DataAvailable += FromPhoneLineDataAvailable;

            ToSpeakerChannel = new WasapiOut(ToSpeakerDevice, DEFAULT_SHARE, DEFAULT_SYNC, DEFAULT_LATENCY);
            ToSpeakerBuffer = new BufferedWaveProvider(FromPhoneLineChannel.WaveFormat);

            // Configure the Mic to Line connection.
            FromMicChannel = new WasapiCapture(FromMicDevice, DEFAULT_SYNC, DEFAULT_LATENCY);
            // FromMicChannel.WaveFormat = ToPhoneLineWaveFormat;
            FromMicChannel.DataAvailable += FromMicDataAvailable;

            LogWaveFormat("ToPhoneLine:      ", ToPhoneLineWaveFormat);
            LogWaveFormat("FromPhoneLine:    ", FromPhoneLineChannel.WaveFormat);
            LogWaveFormat("FromMicChannel:   ", FromMicChannel.WaveFormat);
            LogWaveFormat("ToSpeakerChannel: ", ToSpeakerChannel.OutputWaveFormat);

            ToPhoneLineChannel = new WasapiOut(ToPhoneLineDevice, DEFAULT_SHARE, DEFAULT_SYNC, DEFAULT_LATENCY);

            FromMicBuffer = new BufferedWaveProvider(FromMicChannel.WaveFormat);
            // FromMicBufferFloat = new Wave16ToFloatProvider(FromMicBuffer);

            ToPhoneLineMixer = new MixingWaveProvider32();
            ToPhoneLineMixer.AddInputStream(FromMicBuffer);

            ToSpeakerChannel.Init(ToSpeakerBuffer);
            ToPhoneLineChannel.Init(ToPhoneLineMixer);

            // Start doing work now.
            ToSpeakerChannel.Play();
            ToPhoneLineChannel.Play();
            FromPhoneLineChannel.StartRecording();
            FromMicChannel.StartRecording();

            IsActive = true;
        }

        /*
        public void StartAsio()
        {
            Console.WriteLine("----------------------------");
            Console.WriteLine("ASIO AudioStitcher.Start()");
            var devices = AsioOut.GetDriverNames();
            Console.WriteLine("devices:", devices);
            // NOTE(lbayes): There are no ASIO drivers available by default.
            // Stopped at this point.

            IsActive = true;
        }

        public void StartDso()
        {
            Console.WriteLine("----------------------------");
            Console.WriteLine("DSO AudioStitcher.Start()");

            // Get each of the 4 audio devices by name and data flow.
            FromPhoneLineDevice = GetMMDeviceByName(RxName, DataFlow.Capture);
            ToPhoneLineDso = GetDsoDeviceByName(TxName);

            ToSpeakerDso = GetDsoDeviceByName();
            FromMicDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Capture, Role.Communications);

            FromPhoneLineChannel = new WasapiCapture(FromPhoneLineDevice, DEFAULT_SYNC, DEFAULT_LATENCY);
            FromPhoneLineChannel.WaveFormat = new WaveFormat(44100, 16, 1);
            FromPhoneLineChannel.DataAvailable += FromPhoneLineDataAvailable;

            ToSpeakerDsoChannel = new DirectSoundOut(ToSpeakerDso.Guid);
            ToSpeakerBuffer = new BufferedWaveProvider(FromPhoneLineChannel.WaveFormat);
            ToSpeakerDsoChannel.Init(ToSpeakerBuffer);
            ToSpeakerDsoChannel.Play();
            FromPhoneLineChannel.StartRecording();

            IsActive = true;
        }

        public void StartWave()
        {
            Console.WriteLine("----------------------------");
            Console.WriteLine("Wave AudioStitcher.Start()");
            Console.WriteLine("TxName:" + TxName);
            Console.WriteLine("RxName:" + RxName);

            // Get each of the 4 audio devices by name and data flow.
            FromPhoneLineDevice = GetMMDeviceByName(RxName, DataFlow.Render);
            FromPhoneLineIndex = GetDeviceIndexFor(FromPhoneLineDevice.FriendlyName);

            ToPhoneLineDevice = GetMMDeviceByName(TxName, DataFlow.Capture);
            ToPhoneLineIndex = GetDeviceIndexFor(ToPhoneLineDevice.FriendlyName);

            FromMicDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Capture, Role.Communications);
            FromMicIndex = GetDeviceIndexFor(FromMicDevice.FriendlyName);

            ToSpeakerDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Render, Role.Communications);
            ToSpeakerIndex = GetDeviceIndexFor(ToSpeakerDevice.FriendlyName);

            Console.WriteLine("Microphone: " + FromMicDevice?.FriendlyName);
            Console.WriteLine("Speakers: " + ToSpeakerDevice?.FriendlyName);
            Console.WriteLine("FromPhoneLine: " + FromPhoneLineDevice?.FriendlyName);
            Console.WriteLine("ToPhoneLine: " + ToPhoneLineDevice?.FriendlyName);

            var waveFormat = new WaveFormat();

            // Configure the Line to Speaker connection.
            FromPhoneLineWave = new WaveInEvent();
            FromPhoneLineWave.DeviceNumber = FromPhoneLineIndex;
            FromPhoneLineWave.DataAvailable += FromPhoneLineDataAvailable;

            ToSpeakerWave = new WaveOutEvent();
            ToSpeakerBuffer = new BufferedWaveProvider(FromPhoneLineWave.WaveFormat);

            ToSpeakerWave.Init(ToSpeakerBuffer);

            // Configure the Mic to Line connection.
            FromMicWave = new WaveInEvent();
            FromMicWave.DeviceNumber = FromMicIndex;
            FromMicWave.DataAvailable += FromMicDataAvailable;

            ToPhoneLineWave = new WaveOutEvent();
            ToPhoneLineWave.DeviceNumber = ToPhoneLineIndex;
            ToPhoneLineBuffer = new BufferedWaveProvider(FromMicWave.WaveFormat); //  WaveFormat.CreateIeeeFloatWaveFormat(44100, 2));
            ToPhoneLineWave.Init(ToPhoneLineBuffer);

            // NOTE(lbayes): Tried buffering bits into a shared mixer,
            // but this did not work either.
            // ToPhoneLineMixer = new MixingWaveProvider32();
            // ToPhoneLineMixer.AddInputStream(ToPhoneLineBuffer);

            ToPhoneLineWave.Play();
            ToSpeakerWave.Play();
            FromMicWave.StartRecording();
            FromPhoneLineWave.StartRecording();

            Thread.Sleep(TimeSpan.FromMilliseconds(200));

            IsActive = true;
        }
        */

        public void Stop()
        {
            if (!IsActive)
            {
                return;
            }

            ToSpeakerDsoChannel?.Dispose();

            ToSpeakerWave?.Dispose();
            FromMicWave?.Dispose();
            FromPhoneLineWave?.Dispose();
            ToPhoneLineWave?.Dispose();

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

        private void FromMicDataAvailable32(object sender, WaveInEventArgs e)
        {
            // Console.WriteLine("FROM PHONE LINE DATA AVAIL!");
            float[] buffer = new float[e.BytesRecorded];
            Buffer.BlockCopy(e.Buffer, 0, buffer, 0, e.BytesRecorded);
            // ToPhoneLineBuffer.AddSamples(buffer, 0, e.BytesRecorded);
            // ToPhoneLineMixer.AddSamples(buffer, 0, e.BytesRecorded);
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
