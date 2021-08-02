using System.Collections.Generic;
using Element.CLR;
using ResultNET;

namespace Element
{
    public interface IBoundaryFunctionArguments
    {
        IBoundaryFunction BoundaryFunction { get; }
        Result SetSingle(string path, float value);
        Result SetMultiple(string path, IReadOnlyList<float> values);
        Result<float> GetSingle(string path);
        void ApplyArgumentChanges();
    }
}