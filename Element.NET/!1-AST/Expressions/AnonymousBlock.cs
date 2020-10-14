using Lexico;

namespace Element.AST
{
    public class AnonymousBlock : Expression
    {
#pragma warning disable 8618
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [Term] public CommaSeparatedBlock Block { get; private set; }
#pragma warning restore 8618

        protected override void ValidateImpl(ResultBuilder builder, Context context) => Block.Validate(builder, context);
        protected override Result<IValue> ExpressionImpl(IScope parentScope, Context context) => Block.ResolveBlock(parentScope, context).Cast<IValue>(context);
    }
}