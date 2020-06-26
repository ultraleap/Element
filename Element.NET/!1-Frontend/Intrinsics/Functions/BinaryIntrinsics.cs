using System.Collections.Generic;

namespace Element.AST
{
    /// <summary>
    /// An intrinsic with 2 arguments and a single return.
    /// </summary>
    public sealed class BinaryIntrinsic : IntrinsicFunction
    {
        private static readonly Port[] _numInputs = {
            new Port("a", NumType.Instance),
            new Port("b", NumType.Instance)
        };
        private static readonly Port[] _boolInputs = {
            new Port("a", BoolType.Instance),
            new Port("b", BoolType.Instance)
        };
        
        public static readonly Dictionary<Binary.Op, (Port[] Inputs, IntrinsicType Return)> OperationOverrides = new Dictionary<Binary.Op, (Port[] Inputs, IntrinsicType Return)>
        {
            {Binary.Op.And, (_boolInputs, BoolType.Instance)},
            {Binary.Op.Or, (_boolInputs, BoolType.Instance)},
            
            {Binary.Op.Eq, (_numInputs, BoolType.Instance)},
            {Binary.Op.NEq, (_numInputs, BoolType.Instance)},
            {Binary.Op.Lt, (_numInputs, BoolType.Instance)},
            {Binary.Op.LEq, (_numInputs, BoolType.Instance)},
            {Binary.Op.Gt, (_numInputs, BoolType.Instance)},
            {Binary.Op.GEq, (_numInputs, BoolType.Instance)},
        };

        public BinaryIntrinsic(Binary.Op operation)
        {
            Operation = operation;
            Identifier = new Identifier(operation.ToString().ToLowerInvariant());
            
            // Assumed to be (a:Num, b:Num):Num unless overridden above
            var overridden = OperationOverrides.TryGetValue(operation, out var details);
            Inputs = overridden ? details.Inputs :  new[] {new Port("a", NumType.Instance), new Port("b", NumType.Instance)};
            Output = Port.ReturnPort(overridden ? details.Return : NumType.Instance);
        }

        public Binary.Op Operation { get; }

        public override Identifier Identifier { get; }
        public override IReadOnlyList<Port> Inputs { get; }
        public override Port Output { get; }
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) => new Binary(Operation, (Element.Expression) arguments[0], (Element.Expression) arguments[1]); // TODO: use ApplyArguments
    }
}