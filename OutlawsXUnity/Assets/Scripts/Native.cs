/* 
 * Native dll invocation helper by Francis R. Griffiths-Keam
 * www.runningdimensions.com/blog
 * http://runningdimensions.com/blog/?p=5
 * Modified by Zbigniew Skowron
 */

using System;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Runtime.InteropServices;
using UnityEngine;

public static class Native
{
    public static T Invoke<T, T2>(IntPtr library, params object[] pars)
    {
        IntPtr funcPtr = GetProcAddress(library, typeof(T2).Name);
        if (funcPtr == IntPtr.Zero)
        {
            Debug.LogWarning("Could not gain reference to method address.");
            return default(T);
        }

        var func = Marshal.GetDelegateForFunctionPointer(funcPtr, typeof(T2));
        return (T)func.DynamicInvoke(pars);
    }

    public static void Invoke<T>(IntPtr library, params object[] pars)
    {
        IntPtr funcPtr = GetProcAddress(library, typeof(T).Name);
        if (funcPtr == IntPtr.Zero)
        {
            Debug.LogWarning("Could not gain reference to method address.");
            return;
        }

        var func = Marshal.GetDelegateForFunctionPointer(funcPtr, typeof(T));
        func.DynamicInvoke(pars);
    }

    public static T InvokeSimple<T>(IntPtr library, params object[] pars)
        where T : class
    {
        IntPtr funcPtr = GetProcAddress(library, typeof(T).Name);
        if (funcPtr == IntPtr.Zero)
        {
            throw new ApplicationException("Could not gain reference to method address: " + typeof(T).Name);
        }

        var func = Marshal.GetDelegateForFunctionPointer(funcPtr, typeof(T));
        return func as T;
    }

#if DUMMYS
    public static DelegateT InvokeSimple<DelegateT>(IntPtr library, string name)
        where DelegateT : class
    {
        IntPtr funcPtr = GetProcAddress(library, name);
        if (funcPtr == IntPtr.Zero)
        {
            throw new ApplicationException("Could not gain reference to method address: " + name);
        }

        return Marshal.GetDelegateForFunctionPointer(funcPtr, typeof(DelegateT)) as DelegateT;
    }

    public static ReturnT InvokeDel<ReturnT>(IntPtr library, string name, Type DelegateType, params object[] pars)
    {
        IntPtr funcPtr = GetProcAddress(library, name);
        if (funcPtr == IntPtr.Zero)
        {
            Debug.LogError("Could not gain reference to method address.");
            return default(ReturnT);
        }

        var func = Marshal.GetDelegateForFunctionPointer(funcPtr, DelegateType);
        return (ReturnT)func.DynamicInvoke(pars);
    }

    public static ReturnT InvokeDel<ReturnT>(IntPtr library, Type DelegateType, params object[] pars)
    {
        return InvokeDel<ReturnT>(library, DelegateType.Name, typeof(ReturnT), pars);
    }

    public static ReturnT InvokeDel<ReturnT, DelegateT>(IntPtr library, string name, params object[] pars)
    {
        return InvokeDel<ReturnT>(library, name, typeof(ReturnT), pars);
    }

#if NOFIN
    public static void Invoke<DelegateT>(IntPtr library, params object[] pars)
    {
        IntPtr funcPtr = GetProcAddress(library, typeof(DelegateT).Name);
        if (funcPtr == IntPtr.Zero)
        {
            Debug.LogError("Could not gain reference to method address.");
            return;
        }

        var func = Marshal.GetDelegateForFunctionPointer(funcPtr, typeof(DelegateT));
        func.DynamicInvoke(pars);
    }
#endif

    public static ReturnT Invoke<ReturnT>(IntPtr library, MethodInfo methodInfo, params object[] pars)
    {
        var del = CreateDelegateForStaticMethod(methodInfo);
        return InvokeDel<ReturnT>(library, methodInfo.Name, del.GetType(), pars);
    }

    private static Delegate CreateDelegateForStaticMethod(MethodInfo methodInfo)
    {
        if (!methodInfo.IsStatic)
        {
            throw new ApplicationException("This method can be used on static methods only.");
        }

        Func<Type[], Type> getType;
        var isAction = methodInfo.ReturnType.Equals((typeof(void)));
        var types = methodInfo.GetParameters().Select(p => p.ParameterType);

        if (isAction)
        {
            getType = Expression.GetActionType;
        }
        else
        {
            getType = Expression.GetFuncType;
            types = types.Concat(new[] { methodInfo.ReturnType });
        }

        return Delegate.CreateDelegate(getType(types.ToArray()), methodInfo);
    }
#endif

    [DllImport("kernel32", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool FreeLibrary(IntPtr hModule);

    [DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
    public static extern IntPtr LoadLibrary(string lpFileName);

    [DllImport("kernel32")]
    public static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);
}
