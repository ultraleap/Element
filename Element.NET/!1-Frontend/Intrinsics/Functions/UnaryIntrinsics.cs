using System.Collections.Generic;

namespace Element.AST
{
    internal class UnaryIntrinsic : IntrinsicFunctionSignature
    {
        public UnaryIntrinsic(Unary.Op operation)
        {
            Operation = operation;
            Identifier = new Identifier(operation.ToString().ToLowerInvariant());
        }

        public Unary.Op Operation { get; }

        public override Identifier Identifier { get; }
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
            new Unary(Operation, (Element.Expression) arguments[0]); // TODO: use ApplyArguments
    }
}