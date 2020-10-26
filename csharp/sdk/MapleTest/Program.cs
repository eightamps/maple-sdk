﻿using System;
using System.Threading;
using Maple;

namespace MaplePhoneTest
{
    class Program
    {
        static void Main(string[] args)
        {
            var maple = Phone.First();
            if (maple == null)
            {
                Console.WriteLine("No Maple found!");
                return;
            }

            maple.Dial("15104599053");

            Console.WriteLine("Waiting for 5 mins");
            Thread.Sleep(TimeSpan.FromSeconds(300));

            /*
            Action<Phone, bool> loop = delegate (Phone m, bool set) 
            { Console.WriteLine("loop: " + set); };
            maple.LoopPresence += loop;
            Action<Phone, bool> ring = delegate (Phone m, bool set)
            { Console.WriteLine("ring: " + set); };
            maple.RingingSignal += ring;
            Action<Phone, bool> lineinuse = delegate (Phone m, bool set)
            { Console.WriteLine("lineinuse: " + set); };
            maple.RemoteOffHook += lineinuse;
            Action<Phone, bool> pol = delegate (Phone m, bool set)
            { Console.WriteLine("polarity: " + set); };
            maple.Polarity += pol;

            // Wait a few seconds before attempting to take phone off hook
            Thread.Sleep(3000);
            maple.SetOffHook(true);
            Console.WriteLine("Phone OFF HOOK?");
            Thread.Sleep(TimeSpan.FromSeconds(1));

            maple.Dial("15104599053");

            Thread.Sleep(10000);

            maple.SetOffHook(false);
            Console.WriteLine("Phone ON HOOK?");
            */

            maple.Dispose();
        }
    }
}