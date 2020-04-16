using System;
using Element.AST;

namespace Element
{
    /// <summary>
    /// Host where the compilation context is persisted between commands.
    /// </summary>
    public class PersistentHost : IHost
    {
        public PersistentHost(CompilationInput input)
        {
            if (!SourceContext.TryCreate(input, out _context))
            {
                throw new ArgumentException("Failed to initialize context with given compiler arguments");
            }
        }

        private readonly SourceContext _context;

        public bool Parse(CompilationInput input) => _context.ApplyExtraInput(input);

        public (bool Success, float[] Result) Evaluate(CompilationInput input, string expression) =>
            _context.ApplyExtraInput(input)
                ? _context.EvaluateExpression(expression, out var compilationContext)
                          .TrySerialize(out float[] result, compilationContext)
                      ? (true, result)
                      : (_context.LogError(1, "Result not serializable") == CompilationErr.Instance, null)
                : (false, null);

        public (bool Success, string Result) Typeof(CompilationInput input, string expression) =>
            _context.ApplyExtraInput(input)
                ? _context.EvaluateExpression(expression, out _) switch
                {
                    CompilationErr err => (false, err.ToString()),
                    { } result => (true, result.Type.Name),
                    _ => (false, "<error>")
                }
                : (false, "<compilation input error>");
    }
}