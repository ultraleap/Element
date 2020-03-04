using Lexico;

namespace Element.AST
{
    public interface IExpressionListStart {}

    public interface ISubExpression
    {
        IValue ResolveSubExpression(IValue previous, IScope resolutionScope, CompilationContext compilationContext);
    }

    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public abstract class Expression
    {
        public abstract IValue ResolveExpression(IScope scope, CompilationContext compilationContext);
    }
}