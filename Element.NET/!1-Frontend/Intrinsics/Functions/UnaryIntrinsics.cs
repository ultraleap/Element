using System.Collections.Generic;

namespace Element.AST
{
    internal class UnaryIntrinsic : IntrinsicFunction
    {
        public static readonly Dictionary<Unary.Op, (string Location, Port Input, IType Return)> OperationOverrides = new Dictionary<Unary.Op, (string Location, Port Input, IType Return)>
        {
            {Unary.Op.Not, ("Bool", new Port("a", BoolType.Instance), BoolType.Instance)},
        };

        public UnaryIntrinsic(Unary.Op operation)
            : base(OperationOverrides.TryGetValue(operation, out var details) is {} overridden && overridden
                       ? $"{details.Location}.{operation.ToString().ToLowerInvariant()}"
                       : $"Num.{operation.ToString().ToLowerInvariant()}",
                   new[]{overridden ? details.Input : new Port("a", NumType.Instance)},
                   Port.ReturnPort(overridden ? details.Return : NumType.Instance)) =>
            Operation = operation;
        
        public Unary.Op Operation { get; }

        public override IValue Call(IValue[] arguments, CompilationContext _) => new Unary(Operation, arguments[0] as Element.Expression);
    }
}