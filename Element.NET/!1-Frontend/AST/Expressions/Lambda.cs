using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    public class Lambda : Expression, IFunction
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
        public Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context)
        {
            return FunctionHelpers.ApplyFunction(this, arguments, DeclaringScope, false, context);
        }

        public IScope DeclaringScope { get; private set; }
#pragma warning restore 8618
        
        public IFunction GetDefinition(CompilationContext compilationContext) => this;

        protected override void InitializeImpl(IIntrinsicCache? cache)
        {
            _portList.Initialize(Declarer, cache);
            _type?.Initialize(Declarer, cache);
            Inputs =  _portList.Ports.List.ToArray();
            Output = Port.ReturnPort(_type);
            switch (_body)
            {
                case ExpressionBody b: b.Expression.Initialize(Declarer, cache);
                    break;
                case Scope s: s.Initialize(Declarer, cache);
                    break;
            }
        }

        public override string ToString() => "Lambda";

        public override void Validate(ResultBuilder resultBuilder)
        {
            _portList.Validate(resultBuilder);
            _type?.Validate(resultBuilder);
            switch(_body)
            {
                case ExpressionBody b:
                    b.Expression.Validate(resultBuilder);
                    break;
                case Scope s:
                    s.Validate(resultBuilder, identifierWhitelist: new[] {Parser.ReturnIdentifier});
                    break;
            }
        }

        protected override Result<IValue> ExpressionImpl(IScope scope, CompilationContext compilationContext)
        {
            DeclaringScope = scope;
            return this;
        }
    }
}