﻿using NAudio.CoreAudioApi;
using NAudio.Wave.SampleProviders;
using NAudio.Wave;
using System.Collections.Generic;
using System.Threading;
using System;

namespace Maple
{
    class Dtmf
    {
        private readonly Double DEFAULT_GAIN = 0.9;
        private readonly int DEFAULT_LATENCY = 80;
        private readonly int DEFAULT_TONE_DURATION_MS = 100;

        private Dictionary<char, Tuple<int, int>> DtmfLookup;

        public Dtmf()
        {
        }

        public void GenerateDtmfTones(String phoneNumbers, AudioStitcher stitcher)
        {
            Console.WriteLine("----------------------");
            Console.WriteLine("GenerateTones with:" + phoneNumbers);
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
            Console.WriteLine("GenerateDtmf start");
            foreach (var tone in tones)
            {
                GenerateDtmfTone(stitcher, duration, tone.Item1, tone.Item2);
            }
            Console.WriteLine("GenerateDtmf done");
        }

        private void GenerateDtmfTone(AudioStitcher stitcher, TimeSpan duration, int freq1, int freq2)
        {
            var waveFormat = stitcher.ToPhoneLineWaveFormat;
            var channelCount = waveFormat.Channels;
            Console.Write("SampleRate: " + waveFormat.SampleRate + " channelCount: " + channelCount + " - ");
            var one = new SignalGenerator(waveFormat.SampleRate, channelCount)
            {
                Gain = DEFAULT_GAIN,
                Frequency = freq1,
                Type = SignalGeneratorType.Sin
            };

            var two = new SignalGenerator(waveFormat.SampleRate, channelCount)
            {
                Gain = DEFAULT_GAIN,
                Frequency = freq2,
                Type = SignalGeneratorType.Sin
            };

            var inputs = new List<ISampleProvider>() { one, two };
            var samples = new MixingSampleProvider(inputs);
            // var timedSamples = samples.Take(duration);

            // GenerateWaveOut(stitcher, one, two, duration);
            // GenerateDso(stitcher, timedSamples, duration);
            GenerateWasapi(stitcher, samples, duration); // one, two, duration);
            // GenerateMixerOut(stitcher, timedSamples, duration);
            Console.WriteLine("");
        }

        private void GenerateWasapi(AudioStitcher stitcher, ISampleProvider samples, TimeSpan duration)
        {
            var sampleStream = samples.ToWaveProvider();
            stitcher.ToPhoneLineChannel.Pause();
            stitcher.ToPhoneLineMixer.AddInputStream(sampleStream);
            stitcher.ToPhoneLineChannel.Play();

            Thread.Sleep(duration);

            stitcher.ToPhoneLineChannel.Pause();
            stitcher.ToPhoneLineMixer.RemoveInputStream(sampleStream);
            stitcher.ToPhoneLineChannel.Play();

            Thread.Sleep(TimeSpan.FromMilliseconds(60));
        }

        private void GenerateWaveOut(AudioStitcher stitcher, ISampleProvider freq1, ISampleProvider freq2, TimeSpan duration)
        {
            var deviceNumber = stitcher.ToPhoneLineWave.DeviceNumber;
            Console.WriteLine("Device Index: " + deviceNumber);

            using (var one = new WaveOutEvent())
            using (var two = new WaveOutEvent())
            {
                one.DesiredLatency = DEFAULT_LATENCY;
                one.DeviceNumber = deviceNumber;
                one.Init(freq1.Take(duration));

                two.DesiredLatency = DEFAULT_LATENCY;
                two.DeviceNumber = deviceNumber;
                two.Init(freq2.Take(duration));

                one.Play();
                two.Play();

                while (one.PlaybackState == PlaybackState.Playing ||
                    two.PlaybackState == PlaybackState.Playing)
                {
                    Thread.Sleep(TimeSpan.FromMilliseconds(5));
                }

                Thread.Sleep(TimeSpan.FromMilliseconds(100));
            }
        }

        private void GenerateDso(AudioStitcher stitcher, ISampleProvider samples, TimeSpan duration)
        {
            using (var dso = new DirectSoundOut(stitcher.ToPhoneLineDso.Guid))
            {
                dso.Init(samples);
                dso.Play();

                while (dso.PlaybackState == PlaybackState.Playing)
                {
                    Thread.Sleep(TimeSpan.FromMilliseconds(5));
                }
            }
        }

        private void GenerateMixerOut(AudioStitcher stitcher, ISampleProvider samples, TimeSpan duration)
        {
            Console.WriteLine("Using Mixer Output");
            var count = 10000;
            // byte[] buffer = new byte[count];
            // samples.ToWaveProvider16().Read(buffer, 0, count);
            // stitcher.ToPhoneLineBuffer.AddSamples(buffer, 0, count);

            // stitcher.ToPhoneLineMixer.AddInputStream(samples.ToWaveProvider());
            Thread.Sleep(duration + TimeSpan.FromMilliseconds(10));
        }

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
    }
}
