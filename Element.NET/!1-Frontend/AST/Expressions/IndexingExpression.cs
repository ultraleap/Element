using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class IndexingExpression : SubExpression
    {
#pragma warning disable 169
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [field: Term, Prefix(".")] private Identifier Identifier { get; set; }
#pragma warning restore 169

        public override string ToString() => $".{Identifier}";

        public override void Validate(ResultBuilder resultBuilder) => Identifier.Validate(resultBuilder);
        protected override void InitializeImpl() { /* No-op, nothing to do */ }

        protected override Result<IValue> SubExpressionImpl(IValue previous, IScope _, CompilationContext compilationContext) =>
            previous.Index(Identifier, compilationContext);
    }
}