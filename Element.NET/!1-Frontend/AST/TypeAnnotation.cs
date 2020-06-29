using Lexico;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class TypeAnnotation : Declared
    {
#pragma warning disable 169, 8618
        [Literal(":"), WhitespaceSurrounded, MultiLine] private Unnamed _;

        // ReSharper disable once UnusedAutoPropertyAccessor.Local
#pragma warning disable 
        [field: Term] private Expression Expression { get; set; }
#pragma warning restore 169, 8618

        public override string ToString() => $":{Expression}";

        protected override void InitializeImpl() => Expression.Initialize(Declarer);
        
        public override void Validate(ResultBuilder resultBuilder) {} // TODO: Disallow complex expressions e.g. calls
    }
}