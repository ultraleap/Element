using ResultNET;

namespace Element
{
    public interface IHost
    {
        Result Parse(CompilerInput input);
        Result<float[]> EvaluateExpression(CompilerInput input, string expression, bool interpreted);
        Result<float[]> EvaluateFunction(CompilerInput input, string functionExpression, string argumentsAsCallExpression, bool interpreted);
        Result<string> Typeof(CompilerInput input, string expression);
        Result<string> Summary(CompilerInput input, string expression);
    }
}