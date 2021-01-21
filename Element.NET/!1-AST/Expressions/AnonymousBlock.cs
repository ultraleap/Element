using Lexico;

namespace Element.AST
{
    [Sequence(ParserFlags = ParserFlags.TraceHeader)]
    public class AnonymousBlock : Expression
    {
#pragma warning disable 8618
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [Sequence] public CommaSeparatedBlock Block { get; private set; }
#pragma warning restore 8618

        protected override void ValidateImpl(ResultBuilder builder, Context context) => Block.Validate(builder, context);
        protected override Result<IValue> ExpressionImpl(IScope parentScope, Context context) => Block.ResolveBlock(parentScope, context).Bind(block => StructuralTuple.CreateInstance(block, context));
    }
}