namespace Element
{
    public interface IHost
    {
        bool Parse(CompilationInput input);
        (bool Success, float[] Result) Evaluate(CompilationInput input, string expression);
        (bool Success, string Result) Typeof(CompilationInput input, string expression);
        // Normalized Form
        // Serialized Size
    }
}