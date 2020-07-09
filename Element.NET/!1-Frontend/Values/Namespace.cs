using System.Collections.Generic;

namespace Element.AST
{
    public class Namespace : Value
    {
        private readonly IScope _scope;
        public Namespace(IScope scope, Identifier identifier, string? location = null) : base(location)
        {
            _scope = scope;
            Identifier = identifier;
        }
        
        public override Identifier? Identifier { get; }
        public override IReadOnlyList<Identifier> Members => _scope.Members;
        public override Result<IValue> Index(Identifier id, CompilationContext context) => _scope.Index(id, context);
    }
}