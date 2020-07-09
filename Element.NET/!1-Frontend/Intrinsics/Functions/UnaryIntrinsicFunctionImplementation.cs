using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    internal class UnaryIntrinsicFunctionImplementation : IntrinsicFunctionImplementation
    {
        static UnaryIntrinsicFunctionImplementation()
        {
            Instances = Enum.GetValues(typeof(Unary.Op))
                            .Cast<Unary.Op>()
                            .Select(o => new UnaryIntrinsicFunctionImplementation(o))
                            .ToDictionary(i => i.Operation);
        }
        
        public static IReadOnlyDictionary<Unary.Op, UnaryIntrinsicFunctionImplementation> Instances { get; }
        
        private UnaryIntrinsicFunctionImplementation(Unary.Op operation)
        {
            Operation = operation;
            Identifier = new Identifier(operation.ToString().ToLowerInvariant());
        }

        public Unary.Op Operation { get; }

        public override Identifier Identifier { get; }
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) => new Unary(Operation, (Element.Expression) arguments[0]);
    }
}