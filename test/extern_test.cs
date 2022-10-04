using System;
using System.Text;


namespace jit_test_cs_lib
{
    public static class jit_test_cs
    {
        [return: MarshalAs(UnmanagedType.LPStr)]
        public static string extern_impl_func(
         [MarshalAs(UnmanagedType.I4)] int m,
         [MarshalAs(UnmanagedType.I4)] int n)
        {
            String s = String.Format("m:{0},n:{1}.", m, n);
            return s;
        }
    }
