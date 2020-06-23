using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Port : Declared
    {
#pragma warning disable 649
        // ReSharper disable FieldCanBeMadeReadOnly.Local
        [Alternative(typeof(Identifier), typeof(Unidentifier))] private object _identifier;
        [Optional] private TypeAnnotation? _type;
        // ReSharper restore FieldCanBeMadeReadOnly.Local
#pragma warning restore 649

        public Identifier? Identifier => _identifier is Identifier id ? (Identifier?)id : null;
        private Result<IConstraint> _cachedConstraint;
        
        // ReSharper disable once UnusedMember.Global - Used by Lexico to generate instances
        // ReSharper disable once MemberCanBePrivate.Global
#pragma warning disable 8618
        public Port() {} // Initialize by Lexico
#pragma warning restore 8618
        
        public static Port VariadicPort { get; } = new Port();
        public static Port ReturnPort(TypeAnnotation? annotation) => new Port("return", annotation);
        public static Port ReturnPort(IConstraint constraint) => new Port("return", constraint);

        private Port(string identifier, TypeAnnotation? typeAnnotation) :this(new Identifier(identifier), typeAnnotation) { }

        private Port(Identifier identifier, TypeAnnotation? typeAnnotation)
        {
            _identifier = new Identifier(identifier);
            _type = typeAnnotation;
        }
        
        public Port(string identifier, IConstraint constraint) :this(new Identifier(identifier), constraint) { }
        
        public Port(Identifier identifier, IConstraint constraint)
        {
            _identifier = identifier;
            _cachedConstraint = new Result<IConstraint>(constraint);
        }

        public Result<IConstraint> ResolveConstraint(CompilationContext compilationContext) =>
            ResolveConstraint(compilationContext.SourceContext.GlobalScope, compilationContext);
        
        public Result<IConstraint> ResolveConstraint(IScope scope, CompilationContext compilationContext) =>
            _cachedConstraint.Else(() => _cachedConstraint = _type?.ResolveConstraint(scope, compilationContext) ?? AnyConstraint.Instance);

        public override string ToString() => $"{_identifier}{_type}";

        protected override void InitializeImpl(IIntrinsicCache? cache) => _type?.Initialize(Declarer, cache);

        public override void Validate(ResultBuilder resultBuilder)
        {
            if (_identifier is Identifier id) id.Validate(resultBuilder); // Don't validate identifier if this port has none
            _type?.Validate(resultBuilder);
        }
    }
}