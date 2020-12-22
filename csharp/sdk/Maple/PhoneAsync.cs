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
        public event Action<Phone, bool> RingingChanged;
        public event Action<Phone, bool> HookStateChanged;
        public event Action<Phone, bool> LineIsAvailableChanged;

        public PhoneAsync()
        {
            this.Queue = new Queue<Action<Phone>>();
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
                        phone.RingingChanged += this.PhoneRingingChangedHandler;
                        phone.OffHookChanged += this.PhoneHookStateChangedHandler;
                        phone.LineIsAvailableChanged += this.PhoneLineIsAvailableChangedHandler;

                        while (true)
                        {
                            if (this.Queue.Count > 0)
                            {
                                var operation = this.Queue.Dequeue();
                                operation(phone);
                            }
                            Thread.Sleep(TimeSpan.FromMilliseconds(THREAD_SLEEP_DURATION));
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Unable to attach to Phone: {ex}");
                        return;
                    }
                });
                this.PhoneThread.Start();
            }
        }

        private void PhoneRingingChangedHandler(Phone phone, bool ringingState)
        {
            this.RingingChanged?.Invoke(phone, ringingState);
        }

        private void PhoneHookStateChangedHandler(Phone phone, bool hookState)
        {
            this.HookStateChanged?.Invoke(phone, hookState);
        }

        private void PhoneLineIsAvailableChangedHandler(Phone phone, bool lineIsAvailable)
        {
            this.LineIsAvailableChanged?.Invoke(phone, lineIsAvailable);
        }

        private void Enqueue(Action<Phone> action)
        {
            this.Connect();
            this.Queue.Clear();
            this.Queue.Enqueue(action);
        }

        public void Dial(string phoneNumber, PhoneCallback callback = null)
        {
            this.Enqueue((Phone phone) => 
            {
                Console.WriteLine("YOOOOOOOOOOOOOOOOOOO INSIDE");
                Console.WriteLine("PHONE NUBMER:", phoneNumber);
                if (phone.Dial(phoneNumber))
                {
                    callback?.Invoke(PhoneStatus.SUCCESS, "Call Started with: " + phoneNumber);
                }
                else
                {
                    callback?.Invoke(PhoneStatus.FAILURE, "Unable to dial right now, try again later.");
                }
            });
        }

        public void HangUp(PhoneCallback callback = null)
        {
            this.Enqueue((Phone phone) => 
            {
                Console.WriteLine("HANGUP INSIDE");
                Console.WriteLine("PHONE NUBMER:");
                if (phone.HangUp())
                {
                    callback?.Invoke(PhoneStatus.SUCCESS, "Phone is on hook");
                }
                else
                {
                    callback?.Invoke(PhoneStatus.FAILURE, "Failed to hang up phone");
                }
            });
        }

        public void TakeOffHook(PhoneCallback callback = null)
        {
            this.Enqueue((Phone phone) =>
            {
                if (phone.TakeOffHook())
                {
                    callback?.Invoke(PhoneStatus.SUCCESS, "Phone is off hook");
                }
                else
                {
                    callback?.Invoke(PhoneStatus.FAILURE, "Unable to take off hook");
                }
            });
        }

        public void Dispose()
        {
            if (this.PhoneThread.IsAlive)
            {
                this.PhoneThread.Abort();
                var phone = Phone.First();

                if(phone != null)
                {
                    phone.RingingChanged -= this.PhoneRingingChangedHandler;
                    phone.OffHookChanged -= this.PhoneHookStateChangedHandler;
                    phone.LineIsAvailableChanged -= this.PhoneLineIsAvailableChangedHandler;
                }
            }
        }
    }
}
