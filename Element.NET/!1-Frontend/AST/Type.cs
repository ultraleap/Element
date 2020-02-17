using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    public class Type
    {
        [Literal(":"), WhitespaceSurrounded] private Unnamed _;
        [field: Term] public Identifier Identifier { get; }

        private readonly List<Identifier> _typeIdWhitelist = new List<Identifier> {Parser.AnyTypeIdentifier};
        public bool Validate(CompilationContext compilationContext) => compilationContext.ValidateIdentifier(Identifier, _typeIdWhitelist);
    }
}