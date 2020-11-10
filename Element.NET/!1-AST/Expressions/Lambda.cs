using Lexico;

namespace Element.AST
{
    public class Lambda : Expression
    {
#pragma warning disable 649, 169, 8618
        // ReSharper disable UnassignedField.Global UnusedAutoPropertyAccessor.Local
        [Term] private Unidentifier _;
        [Term] public PortList PortList;
        [Optional] public PortConstraint? ReturnConstraint;
        [Alternative(typeof(ExpressionBody), typeof(FunctionBlock)), WhitespaceSurrounded, MultiLine] public object Body;
        // ReSharper restore UnassignedField.Global UnusedAutoPropertyAccessor.Local
#pragma warning restore 649, 169, 8618

        protected override void ValidateImpl(ResultBuilder builder, Context context)
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

        protected override Result<IValue> ExpressionImpl(IScope parentScope, Context context) =>
            PortList.ResolveFunctionSignature(parentScope, false, false, ReturnConstraint, context)
                    .Map(t => Body switch
                    {
                        // ReSharper disable once RedundantCast
                        ExpressionBody exprBody => (IValue)new ExpressionBodiedFunction(t.InputPorts, t.ReturnPort, exprBody, parentScope),
                        FunctionBlock functionBlock => new ScopeBodiedFunction(t.InputPorts, t.ReturnPort, functionBlock, parentScope),
                        _ => throw new InternalCompilerException($"Unknown function body type '{Body}'")
                    });
    }
}