using System;
using Element.AST;

namespace Element
{
    /// <summary>
    /// Host where the compilation context is persisted between commands.
    /// </summary>
    public class PersistentHost : IHost
    {
        public static bool TryCreate(CompilationInput input, out PersistentHost host)
        {
            host = null;
            if (!SourceContext.TryCreate(input, out var context)) return false;
            host = new PersistentHost(context);
            return true;
        }
        
        private PersistentHost(SourceContext context) => _context = context;

        private readonly SourceContext _context;

        public bool Parse(CompilationInput input) => _context.ApplyExtraInput(input);

        public (bool Success, float[] Result) Evaluate(CompilationInput input, string expression) =>
            _context.ApplyExtraInput(input)
                ? _context.EvaluateExpressionAs<ISerializableValue>(expression, out var compilationContext)
                          ?.Serialize(compilationContext)
                          .ToFloatArray(compilationContext) is {} result
                      ? (true, result)
                      : (_context.LogError(1, "Result not serializable") == CompilationError.Instance, null)
                : (false, Array.Empty<float>());

        public (bool Success, string Result) Typeof(CompilationInput input, string expression) =>
            _context.ApplyExtraInput(input)
                ? _context.EvaluateExpression(expression, out _) switch
                {
                    CompilationError err => (false, err.ToString()),
                    { } result => (true, result.ToString()),
                    null => (false, "<null>")
                }
                : (false, "<compilation input error>");
    }
}