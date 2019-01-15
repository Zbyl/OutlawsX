using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

/// See: https://limbioliong.wordpress.com/2013/11/10/understanding-custom-marshaling-part-3/
/// Usage:
///   [DllImport("TestDLL.dll")]
///   [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(FreeCStrMarshaler))]
///   private static extern string ReturnString();

public class FreeCStrMarshaler : ICustomMarshaler
{
    public object MarshalNativeToManaged(IntPtr pNativeData)
    {
        return Marshal.PtrToStringAnsi(pNativeData);
    }

    public IntPtr MarshalManagedToNative(object ManagedObj)
    {
        return Marshal.StringToCoTaskMemAnsi((string)ManagedObj);
    }

    public void CleanUpNativeData(IntPtr pNativeData)
    {
        Marshal.FreeCoTaskMem(pNativeData);
    }

    public void CleanUpManagedData(object ManagedObj)
    {
        // Nothing to do
    }

    public int GetNativeDataSize()
    {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie)
    {
        return marshaler;
    }

    static private FreeCStrMarshaler marshaler = new FreeCStrMarshaler();
}
