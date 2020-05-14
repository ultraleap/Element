using Lexico;

namespace Element.AST
{
    public class Lambda : Expression
    {
#pragma warning disable 649, 169
        [Term] private Unidentifier _;
        [Term] private PortList _portList;
        [Optional] private TypeAnnotation? _type;
        [Alternative(typeof(ExpressionBody), typeof(Scope)), WhitespaceSurrounded, MultiLine] private object _body;
#pragma warning restore 649, 169

        protected override void InitializeImpl()
        {
            _portList.Initialize(Declarer);
            _type?.Initialize(Declarer);
            switch (_body)
            {
                case ExpressionBody b: b.Expression.Initialize(Declarer);
                    break;
                case Scope s: s.Initialize(Declarer);
                    break;
            }
        }
        public override bool Validate(SourceContext sourceContext)
        {
            var success = true;
            success &= _portList.Validate(sourceContext);
            success &= _type?.Validate(sourceContext) ?? true;
            return success && _body switch
            {
                ExpressionBody b => b.Expression.Validate(sourceContext),
                Scope s => s.ValidateScope(sourceContext, identifierWhitelist: new[] {Parser.ReturnIdentifier}),
                _ => false
            };
        }

        protected override IValue ExpressionImpl(IScope scope, CompilationContext compilationContext) =>
            new AnonymousFunction(scope, _body, _portList, Port.ReturnPort(_type));
    }
}