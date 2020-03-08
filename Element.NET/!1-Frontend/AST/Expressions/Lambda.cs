using Lexico;

namespace Element.AST
{
    public class Lambda : Expression
    {
#pragma warning disable 649, 169
        [Term] private Unidentifier _;
        [Term] private PortList _portList;
        [Optional] private Type? _type;
        [Alternative(typeof(ExpressionBody), typeof(Scope)), WhitespaceSurrounded, MultiLine] private object _body;
#pragma warning restore 649, 169

        public override IValue ResolveExpression(IScope scope, CompilationContext compilationContext) =>
            new AnonymousFunction(scope, _body, _portList, _type);
    }
}