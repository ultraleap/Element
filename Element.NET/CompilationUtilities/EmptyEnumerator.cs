using System;
using System.Collections;
using System.Collections.Generic;

namespace Element
{
    internal class EmptyEnumerator<T> : IEnumerator<T>
    {
        private EmptyEnumerator() {}
        public static IEnumerator<T> Instance { get; } = new EmptyEnumerator<T>();

        public bool MoveNext() => false;

        public void Reset() { }

        public T Current => default;

        object IEnumerator.Current => throw new InvalidOperationException();

        public void Dispose() { }
    }
}