using System;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public class Port
    {
        public Port() {}

        public Port(string identifier, IConstraint? constraint = null)
        {
            _identifier = new Identifier(identifier);
            _cachedConstraint = constraint;
        }
        
#pragma warning disable 649
        [Alternative(typeof(Identifier), typeof(Unidentifier))] private object _identifier;
        [Optional] private Type? _type;
#pragma warning restore 649

        public Identifier? Identifier => _identifier is Identifier id ? (Identifier?)id : null;
        private readonly IConstraint? _cachedConstraint;
        public IConstraint ResolveConstraint(IScope startingScope, CompilationContext compilationContext)
        {
            if (_type == null) return AnyConstraint.Instance;
            if (startingScope == null) throw new ArgumentNullException(nameof(startingScope));

            return _cachedConstraint ?? _type.Expression.ResolveExpression(startingScope, compilationContext) switch
            {
                IConstraint constraint => constraint,
                {} notConstraint => compilationContext.LogError(16, $"'{notConstraint}' is not a constraint"),
                null => throw new ArgumentOutOfRangeException()
            };
        }
        
        public override string ToString() => $"{_identifier}{_type}";
    }
    
    // ReSharper disable once ClassNeverInstantiated.Global
    public class PortList : ListOf<Port> { } // CallExpression looks like a list due to using brackets
}