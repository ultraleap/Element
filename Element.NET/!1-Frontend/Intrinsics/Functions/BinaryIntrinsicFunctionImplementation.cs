using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    /// <summary>
    /// An intrinsic with 2 arguments and a single return.
    /// </summary>
    public sealed class BinaryIntrinsicFunctionImplementation : IntrinsicFunctionImplementation
    {
        static BinaryIntrinsicFunctionImplementation()
        {
            Instances = Enum.GetValues(typeof(Binary.Op))
                                   .Cast<Binary.Op>()
                                   .Select(o => new BinaryIntrinsicFunctionImplementation(o))
                                   .ToDictionary(i => i.Operation);
        }
        
        public static IReadOnlyDictionary<Binary.Op, BinaryIntrinsicFunctionImplementation> Instances { get; }
        
        private BinaryIntrinsicFunctionImplementation(Binary.Op operation)
        {
            Operation = operation;
            Identifier = new Identifier(operation.ToString().ToLowerInvariant());
        }

        public Binary.Op Operation { get; }

        public override Identifier Identifier { get; }
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) => new Binary(Operation, (Element.Expression) arguments[0], (Element.Expression) arguments[1]);
    }
}