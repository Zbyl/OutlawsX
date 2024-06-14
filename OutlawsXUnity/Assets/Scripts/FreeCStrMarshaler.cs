using System;
using System.Runtime.InteropServices;
using UnityEngine;

/// See: https://limbioliong.wordpress.com/2013/11/10/understanding-custom-marshaling-part-3/
/// Usage:
///   [DllImport("TestDLL.dll")]
///   [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(FreeCStrMarshaler))]
///   private static extern string ReturnString();

public class FreeCStrMarshaler : ICustomMarshaler
{
    public object MarshalNativeToManaged(IntPtr pNativeData)
    {
        var str = Marshal.PtrToStringAnsi(pNativeData);
        Debug.Log(string.Format("FreeCStrMarshaler: native to managed: pNativeData={1} -> {0}", str, pNativeData));
        return str;
    }

    public IntPtr MarshalManagedToNative(object ManagedObj)
    {
        var pNativeData = Marshal.StringToCoTaskMemAnsi((string)ManagedObj);
        Debug.Log(string.Format("FreeCStrMarshaler: managed to native: {0} -> pNativeData={1}", (string)ManagedObj, pNativeData));
        return pNativeData;
    }

    public void CleanUpNativeData(IntPtr pNativeData)
    {
        Debug.Log(string.Format("FreeCStrMarshaler: releasing native: pNativeData={0}", pNativeData));
        Marshal.FreeCoTaskMem(pNativeData);
    }

    public void CleanUpManagedData(object ManagedObj)
    {
        // Nothing to do
        Debug.Log(string.Format("FreeCStrMarshaler: releasing managed: {0}", (string)ManagedObj));
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
