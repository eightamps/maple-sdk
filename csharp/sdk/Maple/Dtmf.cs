using NAudio.CoreAudioApi;
using NAudio.Wave.SampleProviders;
using NAudio.Wave;
using System.Collections.Generic;
using System.Threading;
using System;

namespace Maple
{
    class Dtmf
    {
        private readonly Double DEFAULT_GAIN = 0.6;
        private readonly int DEFAULT_TONE_DURATION_MS = 80;

        private Dictionary<char, Tuple<int, int>> DtmfLookup;

        public Dtmf()
        {
        }

        public void GenerateTones(String phoneNumbers, AudioStitcher output)
        {
            // Strip any unsupported characters from the phoneNumbers string.
            string filteredInput = filterPhoneNumbers(phoneNumbers);
            if (filteredInput != phoneNumbers)
            {
                Console.WriteLine("GenerateTones filtered phoneNumbers of: " + phoneNumbers + " to: " + filteredInput);
            }
            else
            {
                Console.WriteLine("GenerateTones with: " + phoneNumbers);
            }

            // Get the device index from the PhoneOutput signal.
            var duration = TimeSpan.FromMilliseconds(DEFAULT_TONE_DURATION_MS);
            // Console.WriteLine("GenerateTones on device: " + output.FriendlyName + " with duration: " + duration.TotalMilliseconds + "ms");
            var tones = StringToDtmf(filteredInput);
            foreach (var tone in tones)
            {
                GenerateDtmf(duration, tone.Item1, tone.Item2, output);
            }
        }

        private void GenerateDtmf(TimeSpan duration, int top, int bottom, AudioStitcher stitcher)
        {
            var waveFormat = stitcher.FromMicChannel.WaveFormat;
            var one = new SignalGenerator(waveFormat.SampleRate, 1)
            {
                Gain = DEFAULT_GAIN,
                Frequency = top,
                Type = SignalGeneratorType.Sin
            };

            var two = new SignalGenerator(waveFormat.SampleRate, 1)
            {
                Gain = DEFAULT_GAIN,
                Frequency = bottom,
                Type = SignalGeneratorType.Sin
            };

            var inputs = new List<ISampleProvider>() { one, two };
            var samples = new MixingSampleProvider(inputs).Take(duration).ToWaveProvider();

            // var sampleCount = 10;

            // byte[] buffer = new byte[sampleCount];
            // stitcher.FromMicBuffer.AddSamples(buffer, 0, sampleCount);

            // var wp = samples.ToWaveProvider();

            // stitcher.FromMicBuffer.AddSamples

            // var waveOut = new WaveOutEvent();
            // waveOut.DeviceNumber = stitcher.ToPhoneLineDevice;
            // waveOut.Init(provider.Take(duration));
            // waveOut.Play();

            /*
            while (waveOut.PlaybackState == PlaybackState.Playing)
            {
                Thread.Sleep(TimeSpan.FromMilliseconds(2));
            }
            */

            /*
            using (var wOut = new WasapiOut(output.ToPhoneLineDevice, AudioClientShareMode.Shared, false, AudioStitcher.DEFAULT_LATENCY))
            {
                wOut.Init(provider.Take(duration));
                wOut.Play();

                while (wOut.PlaybackState == PlaybackState.Playing)
                {
                    Thread.Sleep(TimeSpan.FromMilliseconds(2));
                }
            }
            */
        }

        /*
        private void GenerateDtmf(TimeSpan duration, int top, int bottom, int deviceNumber = -1)
        {
            var one = new SignalGenerator()
            {
                Gain = DEFAULT_GAIN,
                Frequency = top,
                Type = SignalGeneratorType.Sin
            }.Take(duration);

            var two = new SignalGenerator()
            {
                Gain = DEFAULT_GAIN,
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
                    Thread.Sleep(TimeSpan.FromMilliseconds(2));
                }
            }
        }
        */

        private Dictionary<char, Tuple<int, int>> GetLookup()
        {
            if (DtmfLookup == null)
            {
                DtmfLookup = new Dictionary<char, Tuple<int, int>>();
                DtmfLookup.Add('1', Tuple.Create(697, 1209));
                DtmfLookup.Add('2', Tuple.Create(697, 1336));
                DtmfLookup.Add('3', Tuple.Create(697, 1477));
                DtmfLookup.Add('A', Tuple.Create(697, 1633));

                DtmfLookup.Add('4', Tuple.Create(770, 1209));
                DtmfLookup.Add('5', Tuple.Create(770, 1336));
                DtmfLookup.Add('6', Tuple.Create(770, 1477));
                DtmfLookup.Add('B', Tuple.Create(770, 1633));

                DtmfLookup.Add('7', Tuple.Create(852, 1209));
                DtmfLookup.Add('8', Tuple.Create(852, 1336));
                DtmfLookup.Add('9', Tuple.Create(852, 1477));
                DtmfLookup.Add('C', Tuple.Create(852, 1633));

                DtmfLookup.Add('*', Tuple.Create(941, 1209));
                DtmfLookup.Add('0', Tuple.Create(941, 1336));
                DtmfLookup.Add('#', Tuple.Create(941, 1477));
                DtmfLookup.Add('D', Tuple.Create(941, 1633));
            }

            return DtmfLookup;
        }

        /**
         * Build a collection of tone Tuples from the provided string value.
         */
        private Tuple<int, int>[] StringToDtmf(String value)
        {
            var lookup = GetLookup();
            var len = value.Length;
            Tuple<int, int>[] tones = new Tuple<int, int>[len];

            var i = 0;
            foreach (var entry in value.ToCharArray())
            {
                tones[i] = lookup[entry];
                i++;
            }
            return tones;
        }

        /**
         * Take any set of characters and return a new string that includes only those
         * characters that have a known DTMF code (in their original order).
         */
        private string filterPhoneNumbers(String phoneNumbers)
        {
            string whitelist = "0123456789ABCD*#";
            string filteredInput = "";
            foreach (char c in phoneNumbers)
            {
                if (whitelist.IndexOf(c) > -1)
                {
                    filteredInput += c;
                }
            }

            return filteredInput;
        }

        /*
        public void GenerateTones(String phoneNumbers, int deviceNumber)
        {
            // Strip any unsupported characters from the phoneNumbers string.
            string filteredInput = filterPhoneNumbers(phoneNumbers);
            if (filteredInput != phoneNumbers) {
                Console.WriteLine("GenerateTones filtered phoneNumbers of: " + phoneNumbers + " to: " + filteredInput);
            }
            else
            {
                Console.WriteLine("GenerateTones with: " + phoneNumbers);
            }

            // Get the device index from the PhoneOutput signal.
            var duration = TimeSpan.FromMilliseconds(DEFAULT_TONE_DURATION_MS);
            Console.WriteLine("GenerateTones on deviceNumber: " + deviceNumber + " with duration: " + duration.TotalMilliseconds + "ms");
            var tones = StringToDtmf(filteredInput);
            foreach (var tone in tones)
            {
                GenerateDtmf(duration, tone.Item1, tone.Item2, deviceNumber);
            }
        }
        */

    }
}
