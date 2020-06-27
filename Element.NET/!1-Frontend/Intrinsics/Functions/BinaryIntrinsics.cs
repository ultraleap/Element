using System.Collections.Generic;

namespace Element.AST
{
    /// <summary>
    /// An intrinsic with 2 arguments and a single return.
    /// </summary>
    public sealed class BinaryIntrinsic : IntrinsicFunctionSignature
    {
        public BinaryIntrinsic(Binary.Op operation)
        {
            Operation = operation;
            Identifier = new Identifier(operation.ToString().ToLowerInvariant());
        }

        public Binary.Op Operation { get; }

        public override Identifier Identifier { get; }
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
            new Binary(Operation, (Element.Expression) arguments[0], (Element.Expression) arguments[1]); // TODO: use ApplyArguments
    }
}