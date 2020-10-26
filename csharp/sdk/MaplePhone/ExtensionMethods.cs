using HidSharp;
using HidSharp.Reports;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MaplePhone
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
