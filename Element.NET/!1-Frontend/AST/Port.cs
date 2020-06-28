using System;
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
        private Result<IValue> _cachedConstraint;
        private readonly Func<IScope, CompilationContext, Result<IValue>>? _resolveConstraintFunc;
        
        // ReSharper disable once UnusedMember.Global - Used by Lexico to generate instances
        // ReSharper disable once MemberCanBePrivate.Global
#pragma warning disable 8618
        public Port() {} // Initialize by Lexico
#pragma warning restore 8618
        
        public static Port VariadicPort { get; } = new Port();
        public static Port ReturnPort(TypeAnnotation? annotation) => new Port("return", annotation);
        public static Port ReturnPort(IValue constraint) => new Port("return", constraint);
        public static Port ReturnPort(Intrinsic constraint) => new Port("return", constraint);

        private Port(string identifier, TypeAnnotation? typeAnnotation) :this(new Identifier(identifier), typeAnnotation) { }

        private Port(Identifier identifier, TypeAnnotation? typeAnnotation)
        {
            _identifier = new Identifier(identifier);
            _type = typeAnnotation;
        }
        
        private Port(Identifier identifier, Func<IScope, CompilationContext, Result<IValue>> constraintFunc)
        {
            _identifier = new Identifier(identifier);
            _resolveConstraintFunc = constraintFunc;
        }
        
        public Port(string identifier, IValue constraint) :this(new Identifier(identifier), constraint) { }
        public Port(string identifier, Intrinsic intrinsicConstraint)
            : this(new Identifier(identifier), (scope, context) => scope.Lookup(intrinsicConstraint.Identifier, context))
        { }
        
        public Port(Identifier identifier, IValue constraint)
        {
            _identifier = identifier;
            _cachedConstraint = new Result<IValue>(constraint);
        }

        public Result<IValue> ResolveConstraint(CompilationContext compilationContext) =>
            ResolveConstraint(compilationContext.SourceContext.GlobalScope, compilationContext);
        
        public Result<IValue> ResolveConstraint(IScope scope, CompilationContext compilationContext) =>
            _cachedConstraint.Else(() => _cachedConstraint = _type?.ResolveConstraint(scope, compilationContext)
                                                             ?? _resolveConstraintFunc?.Invoke(scope, compilationContext)
                                                             ?? scope.Lookup(AnyConstraint.Instance.Identifier, compilationContext));

        public override string ToString() => $"{_identifier}{_type}";

        protected override void InitializeImpl() => _type?.Initialize(Declarer);

        public override void Validate(ResultBuilder resultBuilder)
        {
            if (_identifier is Identifier id) id.Validate(resultBuilder); // Don't validate identifier if this port has none
            _type?.Validate(resultBuilder);
        }
    }
}