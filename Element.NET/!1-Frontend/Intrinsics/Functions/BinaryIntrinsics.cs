using System.Collections.Generic;

namespace Element.AST
{
    /// <summary>
    /// An intrinsic with 2 arguments and a single return.
    /// </summary>
    public sealed class BinaryIntrinsic : IntrinsicFunctionDeclaration
    {
        private static readonly Port[] _numInputs = {
            new Port("a", NumType.Instance),
            new Port("b", NumType.Instance)
        };
        private static readonly Port[] _boolInputs = {
            new Port("a", BoolType.Instance),
            new Port("b", BoolType.Instance)
        };
        
        public static readonly Dictionary<Binary.Op, (string Location, Port[] Inputs, IIntrinsicType Return)> OperationOverrides = new Dictionary<Binary.Op, (string Location, Port[] Inputs, IIntrinsicType Return)>
        {
            {Binary.Op.And, ("Bool", _boolInputs, BoolType.Instance)},
            {Binary.Op.Or, ("Bool", _boolInputs, BoolType.Instance)},
            
            {Binary.Op.Eq, ("Num", _numInputs, BoolType.Instance)},
            {Binary.Op.NEq, ("Num", _numInputs, BoolType.Instance)},
            {Binary.Op.Lt, ("Num", _numInputs, BoolType.Instance)},
            {Binary.Op.LEq, ("Num", _numInputs, BoolType.Instance)},
            {Binary.Op.Gt, ("Num", _numInputs, BoolType.Instance)},
            {Binary.Op.GEq, ("Num", _numInputs, BoolType.Instance)},
        };

        public BinaryIntrinsic(Binary.Op operation)
            : base(OperationOverrides.TryGetValue(operation, out var details) is {} overridden && overridden
                       ? $"{details.Location}.{operation.ToString().ToLowerInvariant()}"
                       : $"Num.{operation.ToString().ToLowerInvariant()}",
                overridden ? details.Inputs : _numInputs,
                Port.ReturnPort(overridden ? details.Return : NumType.Instance)) =>
            Operation = operation;

        public Binary.Op Operation { get; }

        public override Result<IValue> Call(IValue[] arguments, CompilationContext _) =>
            new Binary(Operation, arguments[0] as Element.Expression, arguments[1] as Element.Expression);
    }
}