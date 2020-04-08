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

        public override void Initialize(Declaration declaration)
        {
            Declarer = declaration;
            _portList.Initialize(declaration);
            _type?.Initialize(declaration);
            /*switch (_body)
            {
                case ExpressionBody b: b.Expression.Initialize(declaration);
                    break;
                case Scope s: s.Initialize(declaration);
                    break;
            }*/
        }

        public override IValue ResolveExpression(CompilationContext compilationContext) => new AnonymousFunction(Declarer, _body, _portList, Port.ReturnPort(_type));
    }
}