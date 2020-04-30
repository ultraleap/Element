using System;
using System.Collections.Generic;

namespace Element
{
    public class LambdaEqualityComparer<T> : IEqualityComparer<T>
    {
        public LambdaEqualityComparer(Func<T, T, bool> comparer, Func<T, int> getHashCode)
        {
            _comparer = comparer;
            _getHashCode = getHashCode;
        }

        private readonly Func<T, T, bool> _comparer;
        private readonly Func<T, int> _getHashCode;

        public bool Equals(T x, T y) => _comparer(x, y);

        public int GetHashCode(T obj) => _getHashCode(obj);
    }
}