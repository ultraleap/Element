using Lexico;

namespace Element.AST
{
    public class Lambda : Expression, IFunctionWithBody
    {
#pragma warning disable 649, 169, 8618
        [Term] private Unidentifier _;
        [Term] private PortList _portList;
        [Optional] private TypeAnnotation? _type;
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [Alternative(typeof(ExpressionBody), typeof(Scope)), WhitespaceSurrounded, MultiLine] private object _body;
#pragma warning restore 649, 169
        
        public Port[] Inputs { get; private set; }
        public Port Output { get; private set; }
        public IScope DeclaringScope { get; private set; }
#pragma warning restore 8618
        
        public IFunctionSignature GetDefinition(CompilationContext compilationContext) => this;

        protected override void InitializeImpl()
        {
            _portList.Initialize(Declarer);
            _type?.Initialize(Declarer);
            Inputs =  _portList.Ports.List.ToArray();
            Output = Port.ReturnPort(_type);
            switch (_body)
            {
                case ExpressionBody b: b.Expression.Initialize(Declarer);
                    break;
                case Scope s: s.Initialize(Declarer);
                    break;
            }
        }

        public override string ToString() => "Lambda";

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

        protected override IValue ExpressionImpl(IScope scope, CompilationContext compilationContext)
        {
            DeclaringScope = scope;
            return this;
        }

        object IFunctionWithBody.Body => _body;
    }
}