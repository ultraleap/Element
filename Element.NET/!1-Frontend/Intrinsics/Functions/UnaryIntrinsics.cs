using System.Collections.Generic;

namespace Element.AST
{
    internal class UnaryIntrinsic : IntrinsicFunction
    {
        public static readonly Dictionary<Unary.Op, (Port Input, IntrinsicType Return)> OperationOverrides = new Dictionary<Unary.Op, (Port Input, IntrinsicType Return)>
        {
            {Unary.Op.Not, (new Port("a", BoolType.Instance), BoolType.Instance)},
        };

        public UnaryIntrinsic(Unary.Op operation)
        {
            Operation = operation;
            Identifier = new Identifier(operation.ToString().ToLowerInvariant());
            
            // Assumed to be (a:Num):Num unless overridden above
            var overridden = OperationOverrides.TryGetValue(operation, out var details);
            Inputs = new[] {overridden ? details.Input : new Port("a", NumType.Instance)};
            Output = Port.ReturnPort(overridden ? details.Return : NumType.Instance);
        }

        public Unary.Op Operation { get; }

        public override Identifier Identifier { get; }
        public override IReadOnlyList<Port> Inputs { get; }
        public override Port Output { get; }
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) => new Unary(Operation, (Element.Expression) arguments[0]); // TODO: use ApplyArguments
    }
}