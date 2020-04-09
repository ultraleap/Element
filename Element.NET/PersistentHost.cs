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
                ? _context.Parse(expression, out AST.Expression expressionObject)
                    ? expressionObject.ResolveExpression(_context.GlobalScope, _context.MakeCompilationContext(out var compilationContext))
                                      .TrySerialize(out float[] result, compilationContext)
                          ? (true, result)
                          : (_context.LogError(1, "Result not serializable") == CompilationErr.Instance, null)
                    : (false, null)
                : (false, null);

        public (bool Success, string Result) Typeof(CompilationInput input, string expression) =>
            _context.ApplyExtraInput(input)
                ? _context.Parse(expression, out AST.Expression expressionObject)
                    ? (true, expressionObject.ResolveExpression(_context.GlobalScope, _context.MakeCompilationContext(out _)).Type.Name)
                    : (false, "<expression parse error>")
                : (false, "<compilation input error>");
    }
}