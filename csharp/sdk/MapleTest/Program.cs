using System;
using System.Threading;
using Maple;

namespace MaplePhoneTest
{
    class Program
    {
        private static PhoneAsync phone = new PhoneAsync();

        static void ExitHandler(object sender, EventArgs e)
        {
            Console.WriteLine("exit");
            phone.Dispose();
        }

        static void InitiatePhoneCall(String input)
        {
            if (input == "")
            {
                input = "(510) 459-9053";
            }

            Console.WriteLine("Dialing:" + input);
            phone.Dial(input, (PhoneStatus status, string message) =>
            {
                Console.WriteLine("Hang Up?");
                Console.ReadLine();
                phone.HangUp((PhoneStatus status, string message) =>
                {
                    Console.WriteLine("HUNG UP with:", status,  message);
                });
            });

        }

        static void testOutputs()
        {
            var control = ControlUnit.First();
            var duration = TimeSpan.FromMilliseconds(200);
            var count = control.GetOutContactCount();
            for (uint index = 0; index < count; index++)
            {
                control.ActivateForDuration(index, duration);
                Thread.Sleep(duration);
            }
        }

        static void Main(string[] args)
        {
            try
            {
                phone = new PhoneAsync();
            } catch (InvalidOperationException)
            {
                Console.WriteLine("Unable to find Phone, is it plugged in and powered on?");
                Console.WriteLine("Exiting now");
                return;
            }

            AppDomain.CurrentDomain.ProcessExit += new EventHandler(ExitHandler);

            while (true)
            {
                Console.WriteLine("------------------------------");
                Console.WriteLine("Press 'a' + Enter to test outputs, or");
                Console.WriteLine("dial a number? (default: 510 459-9053)");
                string input = Console.ReadLine();

                switch (input)
                {
                    case "a":
                        testOutputs();
                        break;
                    default:
                        InitiatePhoneCall(input);
                        break;
                }
            }

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
        }
    }
}
