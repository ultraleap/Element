using System;

namespace Element
{
    public class InternalCompilerException : Exception
    {
        public InternalCompilerException(string message)
            : base(message) { }
    }
}