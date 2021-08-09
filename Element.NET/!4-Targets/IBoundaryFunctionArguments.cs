using System.Collections.Generic;
using ResultNET;

namespace Element
{
    public interface IBoundaryFunctionArguments
    {
        Result SetSingle(string path, float value);
        Result SetMultiple(string path, IReadOnlyList<float> values);
        Result<float> GetSingle(string path);
        void ApplyArgumentChanges();
    }
}