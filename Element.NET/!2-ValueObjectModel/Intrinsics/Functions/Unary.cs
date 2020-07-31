using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    internal class Unary : IntrinsicValue, IIntrinsicFunctionImplementation
    {
        static Unary()
        {
            Instances = Enum.GetValues(typeof(Element.Unary.Op))
                            .Cast<Element.Unary.Op>()
                            .Select(o => new Unary(o))
                            .ToDictionary(i => i.Operation);
        }
        
        public static IReadOnlyDictionary<Element.Unary.Op, Unary> Instances { get; }
        
        private Unary(Element.Unary.Op operation)
        {
            Operation = operation;
            _identifier = new Identifier(operation.ToString().ToLowerInvariant());
        }

        public Element.Unary.Op Operation { get; }
        protected override Identifier _identifier { get; }
        public bool IsVariadic => false;
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) => Element.Unary.CreateAndOptimize(Operation, (Element.Expression) arguments[0]);
    }
}