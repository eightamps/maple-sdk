using NAudio.CoreAudioApi;
using NAudio.Wave;
using System;

namespace Maple
{
    public class AudioStitcher : IDisposable
    {
        public const int DEFAULT_LATENCY = 10;

        public String RxName { get; private set; }
        public String TxName { get; private set; }

        public bool IsActive { get; private set; }

        public int FromPhoneLineIndex { get; private set; }
        public int ToPhoneLineIndex { get; private set; }
        public int ToSpeakerIndex { get; private set; }
        public int FromMicIndex { get; private set; }

        public WaveFormat FromPhoneLineWaveFormat { get; private set; }

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

        public void StartDso()
        {
            if (IsActive)
            {
                return;
            }
            Console.WriteLine("----------------------------");
            Console.WriteLine("DSO AudioStitcher.Start()");

            // Get each of the 4 audio devices by name and data flow.
            FromPhoneLineDevice = GetMMDeviceByName(RxName, DataFlow.Capture);
            ToPhoneLineDso = GetDsoDeviceByName(TxName);

            ToSpeakerDso = GetDsoDeviceByName();
            FromMicDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Capture, Role.Communications);

            FromPhoneLineChannel = new WasapiCapture(FromPhoneLineDevice, true);
            FromPhoneLineChannel.WaveFormat = new WaveFormat(44100, 16, 1);
            FromPhoneLineChannel.DataAvailable += FromPhoneLineDataAvailable;
            FromPhoneLineWaveFormat = FromPhoneLineChannel.WaveFormat;

            ToSpeakerDsoChannel = new DirectSoundOut(ToSpeakerDso.Guid);
            ToSpeakerBuffer = new BufferedWaveProvider(FromPhoneLineChannel.WaveFormat);
            ToSpeakerDsoChannel.Init(ToSpeakerBuffer);
            ToSpeakerDsoChannel.Play();
            FromPhoneLineChannel.StartRecording();

            IsActive = true;
        }

        public void StartWasapi()
        {
            if (IsActive)
            {
                return;
            }
            Console.WriteLine("----------------------------");
            Console.WriteLine("Wasapi AudioStitcher.Start()");

            // Get each of the 4 audio devices by name and data flow.
            FromPhoneLineDevice = GetMMDeviceByName(RxName, DataFlow.Capture);
            ToPhoneLineDevice = GetMMDeviceByName(TxName, DataFlow.Render);

            ToSpeakerDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Render, Role.Communications);
            FromMicDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Capture, Role.Communications);

            // Configure the Line to Speaker connection.
            FromPhoneLineChannel = new WasapiCapture(FromPhoneLineDevice, true);
            FromPhoneLineChannel.WaveFormat = new WaveFormat(44100, 16, 1);
            FromPhoneLineChannel.DataAvailable += FromPhoneLineDataAvailable;
            FromPhoneLineWaveFormat = FromPhoneLineChannel.WaveFormat;

            ToSpeakerChannel = new WasapiOut(ToSpeakerDevice, AudioClientShareMode.Shared, true, DEFAULT_LATENCY);
            ToSpeakerBuffer = new BufferedWaveProvider(FromPhoneLineChannel.WaveFormat);
            ToSpeakerChannel.Init(ToSpeakerBuffer);
            ToSpeakerChannel.Play();
            FromPhoneLineChannel.StartRecording();

            // Configure the Mic to Line connection.
            FromMicChannel = new WasapiCapture(FromMicDevice, true);
            FromMicChannel.DataAvailable += FromMicDataAvailable;

            ToPhoneLineChannel = new WasapiOut(ToPhoneLineDevice, AudioClientShareMode.Shared, true, DEFAULT_LATENCY);
            ToPhoneLineBuffer = new BufferedWaveProvider(FromMicChannel.WaveFormat);
            ToPhoneLineChannel.Init(ToPhoneLineBuffer);
            ToPhoneLineChannel.Play();
            FromMicChannel.StartRecording();

            IsActive = true;
        }

        public void StartWave()
        {
            if (IsActive)
            {
                return;
            }

            Console.WriteLine("----------------------------");
            Console.WriteLine("Wave AudioStitcher.Start()");
            Console.WriteLine("TxName:" + TxName);
            Console.WriteLine("RxName:" + RxName);

            // Get each of the 4 audio devices by name and data flow.
            FromPhoneLineDevice = GetMMDeviceByName(TxName, DataFlow.Capture);
            FromPhoneLineIndex = GetDeviceIndexFor(TxName);
            ToPhoneLineDevice = GetMMDeviceByName(RxName, DataFlow.Render);
            ToPhoneLineIndex = GetDeviceIndexFor(RxName);

            ToSpeakerDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Render, Role.Communications);
            ToSpeakerIndex = GetDeviceIndexFor(ToSpeakerDevice.FriendlyName);

            FromMicDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Capture, Role.Communications);
            FromMicIndex = GetDeviceIndexFor(FromMicDevice.FriendlyName);

            Console.WriteLine("Microphone: " + FromMicDevice?.FriendlyName);
            Console.WriteLine("Speakers: " + ToSpeakerDevice?.FriendlyName);
            Console.WriteLine("FromPhoneLine: " + FromPhoneLineDevice?.FriendlyName);
            Console.WriteLine("ToPhoneLine: " + ToPhoneLineDevice?.FriendlyName);

            // Configure the Line to Speaker connection.
            FromPhoneLineWave = new WaveInEvent();
            FromPhoneLineWave.DeviceNumber = FromPhoneLineIndex;
            FromPhoneLineWave.DataAvailable += FromPhoneLineDataAvailable;
            FromPhoneLineWaveFormat = FromPhoneLineWave.WaveFormat;

            ToSpeakerWave = new WaveOutEvent();
            ToSpeakerBuffer = new BufferedWaveProvider(FromPhoneLineWave.WaveFormat);

            ToSpeakerWave.Init(ToSpeakerBuffer);
            ToSpeakerWave.Play();
            FromPhoneLineWave.StartRecording();

            // Configure the Mic to Line connection.
            FromMicWave = new WaveInEvent();
            FromMicWave.DeviceNumber = FromMicIndex;
            FromMicWave.DataAvailable += FromMicDataAvailable;

            ToPhoneLineWave = new WaveOutEvent();
            ToPhoneLineWave.DeviceNumber = ToPhoneLineIndex;
            ToPhoneLineBuffer = new BufferedWaveProvider(FromMicWave.WaveFormat);
            ToPhoneLineWave.Init(ToPhoneLineBuffer);
            ToPhoneLineWave.Play();
            FromMicWave.StartRecording();

            IsActive = true;
        }

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

        private void FromMicDataAvailable(object sender, WaveInEventArgs e)
        {
            // Console.WriteLine("FROM PHONE LINE DATA AVAIL!");
            byte[] buffer = new byte[e.BytesRecorded];
            Buffer.BlockCopy(e.Buffer, 0, buffer, 0, e.BytesRecorded);
            ToPhoneLineBuffer.AddSamples(buffer, 0, e.BytesRecorded);
        }

        private int GetDeviceIndexFor(String name)
        {
            var enumerator = new MMDeviceEnumerator();
            var index = 0;
            foreach (var device in enumerator.EnumerateAudioEndPoints(DataFlow.Render, DeviceState.Active))
            {
                if (device.FriendlyName == name)
                {
                    Console.WriteLine("Found requested device at index: " + index);
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
                if (name == "" || device.Description == name) 
                {
                    Console.WriteLine("FOUND MATCH:" + name);
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
                        Console.WriteLine("GetDeviceWithProductName friendlyName: " + device.FriendlyName + " direction: " + direction);
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
