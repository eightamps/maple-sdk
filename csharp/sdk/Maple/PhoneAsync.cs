using System;
using System.Collections.Generic;
using System.Threading;

namespace Maple
{
    public enum PhoneStatus
    {
        SUCCESS,
        FAILURE,
    }

    public class PhoneAsync : IDisposable
    {
        public delegate void PhoneCallback(PhoneStatus status, string message);

        private Thread PhoneThread;
        private Queue<Action<Phone>> Queue;

        private const int THREAD_SLEEP_DURATION = 10;

        public PhoneAsync()
        {
            this.Connect();
        }

        public void Connect()
        {
            if (this.PhoneThread == null || !this.PhoneThread.IsAlive)
            {
                this.PhoneThread = new Thread(() =>
                {
                    Phone phone;
                    try
                    {
                        phone = Phone.First();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Unable to attach to Phone: {ex}");
                        return;
                    }

                    while (true)
                    {
                        if (this.Queue.Count > 0)
                        {
                            var operation = this.Queue.Dequeue();
                            operation(phone);
                        }
                        Thread.Sleep(TimeSpan.FromMilliseconds(THREAD_SLEEP_DURATION));
                    }

                });
                this.PhoneThread.Start();
            }
        }

        private void Enqueue(Action<Phone> action)
        {
            this.Connect();
            this.Queue.Enqueue(action);
        }

        public void Dial(string phoneNumber, PhoneCallback callback)
        {
            this.Enqueue((Phone phone) => 
            {
                Console.WriteLine("YOOOOOOOOOOOOOOOOOOO INSIDE");
                Console.WriteLine("PHONE NUBMER:", phoneNumber);
                phone.Dial(phoneNumber);
                callback(PhoneStatus.SUCCESS, "Call Started with: " + phoneNumber);
            });
        }

        public void HangUp(PhoneCallback callback)
        {
            this.Queue.Enqueue((Phone phone) => 
            {
                Console.WriteLine("HANGUP INSIDE");
                Console.WriteLine("PHONE NUBMER:");
                phone.HangUp();
                callback(PhoneStatus.SUCCESS, "Hung Up Dude");
            });
        }

        public void Dispose()
        {
            if (this.PhoneThread.IsAlive)
            {
                this.PhoneThread.Abort();
            }
        }
    }
}
