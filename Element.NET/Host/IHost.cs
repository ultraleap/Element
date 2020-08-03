namespace Element
{
    public interface IHost
    {
        Result Parse(CompilerInput input);
        Result<float[]> Evaluate(CompilerInput input, string expression);
        Result<string> Typeof(CompilerInput input, string expression);
    }
}