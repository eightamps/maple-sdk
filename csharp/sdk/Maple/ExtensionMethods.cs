using HidSharp;

namespace Maple
{
    static class ExtensionMethods
    {
        public static bool IsMapleControlDevice(this HidDevice hiddev)
        {
            if (hiddev == null)
                return false;
            if (hiddev.VendorID != 0x335e)
                return false;
            if (hiddev.ProductID != 0x8a01)
                return false;
            return true;
        }
    }
}
