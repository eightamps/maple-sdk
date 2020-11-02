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

        public MMDevice ToPhoneLineDevice { get; private set; }
        public MMDevice FromPhoneLineDevice { get; private set; }

        public MMDevice ToSpeakerDevice { get; private set; }
        public MMDevice FromMicDevice { get; private set; }

        public WasapiCapture FromMicChannel { get; private set; }
        public WasapiOut ToPhoneLineChannel { get; private set; }

        public WasapiCapture FromPhoneLineChannel { get; private set; }
        public WasapiOut ToSpeakerChannel { get; private set; }

        public BufferedWaveProvider FromPhoneLineBuffer { get; private set; }
        public BufferedWaveProvider FromMicBuffer { get; private set; }

        public int SampleRate
        {
            get { return (int)FromMicChannel?.WaveFormat.SampleRate; }
        }

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
            Console.WriteLine("----------------------------");
            Console.WriteLine("AudioStitcher.Start()");

            // Get each of the 4 audio devices by name and data flow.
            FromPhoneLineDevice = GetDeviceWithProductName(RxName, DataFlow.Capture);
            ToPhoneLineDevice = GetDeviceWithProductName(TxName, DataFlow.Render);

            ToSpeakerDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Render, Role.Communications);
            FromMicDevice = new MMDeviceEnumerator().GetDefaultAudioEndpoint(DataFlow.Capture, Role.Communications);

            // Configure the Line to Speaker connection.
            FromPhoneLineChannel = new WasapiCapture(FromPhoneLineDevice, true);
            FromPhoneLineChannel.DataAvailable += FromPhoneLineDataAvailable;
            FromPhoneLineBuffer = new BufferedWaveProvider(FromPhoneLineChannel.WaveFormat);

            ToSpeakerChannel = new WasapiOut(ToSpeakerDevice, AudioClientShareMode.Shared, true, DEFAULT_LATENCY);
            ToSpeakerChannel.Init(FromPhoneLineBuffer);
            ToSpeakerChannel.Play();
            FromPhoneLineChannel.StartRecording();

            // Configure the Mic to Line connection.
            FromMicChannel = new WasapiCapture(FromMicDevice, true);
            FromMicChannel.DataAvailable += FromMicDataAvailable;
            FromMicBuffer = new BufferedWaveProvider(FromMicChannel.WaveFormat);

            ToPhoneLineChannel = new WasapiOut(ToPhoneLineDevice, AudioClientShareMode.Shared, true, DEFAULT_LATENCY);
            ToPhoneLineChannel.Init(FromMicBuffer);
            ToPhoneLineChannel.Play();
            FromMicChannel.StartRecording();

            IsActive = true;
        }

        public void Stop()
        {
            if (!IsActive)
            {
                return;
            }

            ToSpeakerChannel?.Dispose();
            FromPhoneLineChannel?.Dispose();
            FromMicChannel?.Dispose();
            ToPhoneLineChannel?.Dispose();
            IsActive = false;
        }

        private void FromPhoneLineDataAvailable(object sender, WaveInEventArgs e)
        {
            // Console.WriteLine("FROM PHONE LINE DATA AVAIL!");
            byte[] buffer = new byte[e.BytesRecorded];
            Buffer.BlockCopy(e.Buffer, 0, buffer, 0, e.BytesRecorded);
            FromPhoneLineBuffer.AddSamples(buffer, 0, e.BytesRecorded);
        }

        private void FromMicDataAvailable(object sender, WaveInEventArgs e)
        {
            // Console.WriteLine("FROM PHONE LINE DATA AVAIL!");
            byte[] buffer = new byte[e.BytesRecorded];
            Buffer.BlockCopy(e.Buffer, 0, buffer, 0, e.BytesRecorded);
            FromMicBuffer.AddSamples(buffer, 0, e.BytesRecorded);
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
            if (IsActive)
            {
                Stop();
            }
        }
    }
}
