using System.IO;
using Element.AST;

namespace Element
{
    /// <summary>
    /// Host where each command executes in a temporary compilation context and no compilation state is persisted between commands.
    /// </summary>
    public class AtomicHost : IHost
    {
        public bool ParseFile(CompilationInput compilationInput, FileInfo file) =>
            CompilationContext.TryCreate(compilationInput, out var context) && context.ParseFile(file);

        public (bool Success, float[] Result) Evaluate(CompilationInput compilationInput, string expression) =>
            CompilationContext.TryCreate(compilationInput, out var context)
                ? context.Parse(expression, out AST.Expression expressionObject)
                    ?
                    expressionObject.ResolveExpression(context.GlobalScope, context) switch
                    {
                        ISerializable serializable => (serializable.TrySerialize(out var result), result),
                        _ => (context.LogError(1, "Result not serializable") == CompilationErr.Instance, null)
                    }
                    : (false, null)
                : (false, null);

        public (bool Success, string Result) Typeof(CompilationInput input, string expression) =>
            CompilationContext.TryCreate(input, out var context)
                ? context.Parse(expression, out AST.Expression expressionObject)
                    ? (true, expressionObject.ResolveExpression(context.GlobalScope, context).Type.Name)
                : (false, "<expression parse error>")
            : (false, "<source parse error>");
    }
}