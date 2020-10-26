using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    /// <summary>
    /// An intrinsic with 2 arguments and a single return.
    /// </summary>
    public sealed class Binary : IntrinsicValue, IIntrinsicFunctionImplementation
    {
        static Binary()
        {
            Instances = Enum.GetValues(typeof(Element.Binary.Op))
                                   .Cast<Element.Binary.Op>()
                                   .Select(o => new Binary(o))
                                   .ToDictionary(i => i.Operation);
        }
        
        public static IReadOnlyDictionary<Element.Binary.Op, Binary> Instances { get; }
        
        private Binary(Element.Binary.Op operation)
        {
            Operation = operation;
            Identifier = new Identifier(operation.ToString().ToLowerInvariant());
        }

        public Element.Binary.Op Operation { get; }
        public override Identifier Identifier { get; }
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) => Element.Binary.CreateAndOptimize(Operation, arguments[0], arguments[1], context);
        public bool IsVariadic => false;
    }
}