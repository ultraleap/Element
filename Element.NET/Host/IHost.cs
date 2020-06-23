namespace Element
{
    public interface IHost
    {
        Result Parse(CompilationInput input);
        Result<float[]> Evaluate(CompilationInput input, string expression);
        Result<string> Typeof(CompilationInput input, string expression);
        // Normalized Form
        // Serialized Size
    }
}