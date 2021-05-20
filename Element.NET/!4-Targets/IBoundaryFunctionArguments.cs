using ResultNET;

namespace Element
{
    public interface IBoundaryFunctionArguments
    {
        Result SetSingle(string path, float value);
        Result<float> GetSingle(string path);
        //Result Set<TValue>(string path, TValue value);
        //Result<TValue> Get<TValue>(string path);
        void ApplyArgumentChanges();
    }
}