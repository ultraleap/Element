using System;
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

        public Port(string identifier, TypeAnnotation type)
        {
            _identifier = new Identifier(identifier);
            _type = type;
        }

        public static Port VariadicPort { get; } = new Port();
        
#pragma warning disable 649
        // ReSharper disable FieldCanBeMadeReadOnly.Local
        [Alternative(typeof(Identifier), typeof(Unidentifier))] private object _identifier;
        [Optional] private TypeAnnotation? _type;
        // ReSharper restore FieldCanBeMadeReadOnly.Local
#pragma warning restore 649

        public Identifier? Identifier => _identifier is Identifier id ? (Identifier?)id : null;
        public IConstraint ResolveConstraint(IScope startingScope, CompilationContext compilationContext) =>
            (_type, startingScope) switch
            {
                (TypeAnnotation t, IScope scope) => t.ResolveConstraint(scope, compilationContext),
                (null, _) => AnyConstraint.Instance,
                (_, null) => throw new ArgumentNullException(nameof(startingScope))
            };

        public override string ToString() => $"{_identifier}{_type}";
    }
    
    // ReSharper disable once ClassNeverInstantiated.Global
    public class PortList : ListOf<Port> { } // CallExpression looks like a list due to using brackets
}