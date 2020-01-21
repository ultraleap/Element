using System;
using System.Collections.Generic;

namespace Element
{
    internal static class CompilationExtensions
    {
        public static Stack<T> Clone<T>(this Stack<T> original)
        {
            var arr = new T[original.Count];
            original.CopyTo(arr, 0);
            Array.Reverse(arr);
            return new Stack<T>(arr);
        }
    }
}