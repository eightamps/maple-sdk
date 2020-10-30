﻿using NAudio.Wave.SampleProviders;
using NAudio.Wave;
using System.Collections.Generic;
using System.Threading;
using System;

namespace Maple
{
    class Dtmf
    {
        private Dictionary<char, Tuple<int, int>> DtmfLookup;

        private int DeviceNumber = -1;

        public Dtmf(int deviceNumber)
        {
            DeviceNumber = deviceNumber;
        }

        private void GenerateDtmf(TimeSpan duration, int top, int bottom, int deviceNumber = -1)
        {
            var one = new SignalGenerator()
            {
                Gain = 0.3,
                Frequency = top,
                Type = SignalGeneratorType.Sin
            }.Take(duration);

            var two = new SignalGenerator()
            {
                Gain = 0.3,
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
                    Thread.Sleep(TimeSpan.FromMilliseconds(10));
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
        private string filterInput(String input)
        {
            string whitelist = "0123456789ABCD*#";
            string filteredInput = "";
            foreach (char c in input)
            {
                if (whitelist.IndexOf(c) > -1)
                {
                    filteredInput += c;
                }
            }

            return filteredInput;
        }


        public void GenerateTones(String input)
        {
            // Strip any unsupported characters from the input string.
            string filteredInput = filterInput(input);
            if (filteredInput != input) {
                Console.WriteLine("GenerateTones filtered input of: " + input + " to: " + filteredInput);
            }
            else
            {
                Console.WriteLine("GenerateTones with: " + input);
            }

            // Get the device index from the PhoneOutput signal.
            Console.WriteLine("GENERATE TONES FOR:" + DeviceNumber);
            var duration = TimeSpan.FromMilliseconds(200);
            var tones = StringToDtmf(filteredInput);
            foreach (var tone in tones)
            {
                GenerateDtmf(duration, tone.Item1, tone.Item2, DeviceNumber);
            }
        }

    }
}
