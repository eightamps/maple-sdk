using System;
using System.Threading;
using MaplePhone;

namespace MaplePhoneTest
{
    class Program
    {
        static void Main(string[] args)
        {
            var maple = MaplePhoneControl.First();
            if (maple == null)
            {
                Console.WriteLine("No Maple found!");
                return;
            }


            Action<MaplePhoneControl, bool> loop = delegate (MaplePhoneControl m, bool set) 
            { Console.WriteLine("loop: " + set); };
            maple.LoopPresence += loop;
            Action<MaplePhoneControl, bool> ring = delegate (MaplePhoneControl m, bool set)
            { Console.WriteLine("ring: " + set); };
            maple.RingingSignal += ring;
            Action<MaplePhoneControl, bool> lineinuse = delegate (MaplePhoneControl m, bool set)
            { Console.WriteLine("lineinuse: " + set); };
            maple.RemoteOffHook += lineinuse;
            Action<MaplePhoneControl, bool> pol = delegate (MaplePhoneControl m, bool set)
            { Console.WriteLine("polarity: " + set); };
            maple.Polarity += pol;

            //maple.SetOffHook(true);

            Thread.Sleep(10000000);

            //maple.SetOffHook(false);

            maple.Dispose();
        }
    }
}
