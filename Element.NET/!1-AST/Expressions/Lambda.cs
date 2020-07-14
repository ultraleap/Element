using Lexico;

namespace Element.AST
{
    public class Lambda : Expression
    {
#pragma warning disable 649, 169, 8618
        // ReSharper disable UnassignedField.Global UnusedAutoPropertyAccessor.Local
        [Term] private Unidentifier _;
        [Term] public PortList PortList;
        [Optional] public ReturnConstraint? ReturnConstraint;
        [Alternative(typeof(ExpressionBody), typeof(FunctionBlock)), WhitespaceSurrounded, MultiLine] public object Body;
        // ReSharper restore UnassignedField.Global UnusedAutoPropertyAccessor.Local
#pragma warning restore 649, 169, 8618

        public override string ToString() => "Lambda";

        protected override void ValidateImpl(ResultBuilder builder, CompilationContext context)
        {
            PortList.Validate(builder, context);
            ReturnConstraint?.Validate(builder, context);
            switch(Body)
            {
                case ExpressionBody exprBody:
                    exprBody.Expression.Validate(builder, context);
                    break;
                case FunctionBlock block:
                    block.Validate(builder, context);
                    block.ValidateIdentifiers(builder);
                    break;
            }
        }

        protected override Result<IValue> ExpressionImpl(IScope scope, CompilationContext compilationContext) =>
            PortList.ResolveInputConstraints(scope, compilationContext, false, false)
                    .Accumulate(() => ReturnConstraint.ResolveReturnConstraint(scope, compilationContext))
                    .Map(t =>
                    {
                        var (inputPorts, returnConstraint) = t;
                        return Body switch
                        {
                            // ReSharper disable once RedundantCast
                            ExpressionBody exprBody => (IValue)new ExpressionBodiedFunction(inputPorts, returnConstraint, exprBody, scope, compilationContext.CurrentDeclarationLocation),
                            FunctionBlock functionBlock => new ScopeBodiedFunction(inputPorts, returnConstraint, functionBlock, scope, compilationContext.CurrentDeclarationLocation),
                            _ => throw new InternalCompilerException($"Unknown function body type '{Body}'")
                        };
                    });
    }
}