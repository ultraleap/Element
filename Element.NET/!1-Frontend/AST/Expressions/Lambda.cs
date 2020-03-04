using Lexico;

namespace Element.AST
{
    public class Lambda : Expression
    {
        [Literal("_")] private Unnamed _;
        [Term] private PortList _portList;
        [Optional] private Type? _type;
        [Alternative(typeof(Binding), typeof(Scope)), WhitespaceSurrounded, MultiLine] private object _body;

        public override IValue ResolveExpression(IScope scope, CompilationContext compilationContext) =>
            new AnonymousFunction(scope, _body, _portList, _type);
    }
}