using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Port
    {
        // ReSharper disable once UnusedMember.Global - Used by Lexico to generate instances
        // ReSharper disable once MemberCanBePrivate.Global
        public Port() {}

        public Port(string identifier, TypeAnnotation? typeAnnotation) :this(new Identifier(identifier), typeAnnotation) { }
        
        public Port(Identifier identifier, TypeAnnotation? typeAnnotation)
        {
            _identifier = new Identifier(identifier);
            _type = typeAnnotation;
        }
        
        public Port(string identifier, IConstraint constraint) :this(new Identifier(identifier), constraint) { }
        
        public Port(Identifier identifier, IConstraint constraint)
        {
            _identifier = identifier;
            _cachedConstraint = constraint;
        }
        
        public static Port VariadicPort { get; } = new Port();
        public static Port ReturnPort(TypeAnnotation? annotation) => new Port("return", annotation);
        public static Port ReturnPort(IConstraint constraint) => new Port("return", constraint);
        
#pragma warning disable 649
        // ReSharper disable FieldCanBeMadeReadOnly.Local
        [Alternative(typeof(Identifier), typeof(Unidentifier))] private object _identifier;
        [Optional] private TypeAnnotation? _type;
        // ReSharper restore FieldCanBeMadeReadOnly.Local
#pragma warning restore 649

        public Identifier? Identifier => _identifier is Identifier id ? (Identifier?)id : null;
        private IConstraint? _cachedConstraint;

        public IConstraint ResolveConstraint(CompilationContext compilationContext) =>
            ResolveConstraint(compilationContext.SourceContext.GlobalScope, compilationContext);
        
        public IConstraint ResolveConstraint(IScope scope, CompilationContext compilationContext) =>
            _cachedConstraint ?? (_type != null
                                      ? _cachedConstraint = _type.ResolveConstraint(scope, compilationContext)
                                      : AnyConstraint.Instance);

        public Port CloneWithNewId(Identifier id)
        {
            var clone = (Port)MemberwiseClone();
            clone._identifier = id;
            return clone;
        }

        public override string ToString() => $"{_identifier}{_type}";
    }
    
    // ReSharper disable once ClassNeverInstantiated.Global
    public class PortList : ListOf<Port> // CallExpression uses ListOf because it looks like a list due to using brackets
    { }
}