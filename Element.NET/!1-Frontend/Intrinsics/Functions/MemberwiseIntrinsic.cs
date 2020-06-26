using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class MemberwiseIntrinsic : IntrinsicFunction
    {
        private MemberwiseIntrinsic()
        {
            Identifier =  new Identifier("memberwise");
            Inputs = new[]
            {
                new Port("function", FunctionConstraint.Instance),
                Port.VariadicPort
            };
            Output = Port.ReturnPort(AnyConstraint.Instance);
        }
        public static MemberwiseIntrinsic Instance { get; } = new MemberwiseIntrinsic();

        public override Identifier Identifier { get; }
        public override IReadOnlyList<Port> Inputs { get; }
        public override Port Output { get; }
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context)
        {
            var func = (IFunction)arguments[0];
            var structs = arguments.Skip(1).Select(arg => arg as StructInstance).ToArray();
            if (structs.Any(s => s == null)) return context.Trace(MessageCode.ConstraintNotSatisfied, "Memberwise can only be applied to struct instances");
            if (structs.Length < 1) return context.Trace(MessageCode.ConstraintNotSatisfied, "Memberwise requires at least 1 struct to apply the given function");
            
            var structType = structs[0]!.DeclaringStruct;
            if (structType.Fields.Any(f => !f.Identifier.HasValue)) return context.Trace(MessageCode.ConstraintNotSatisfied, "Memberwise cannot be applied to structs with anonymous fields");
            if (structs.Any(s => s!.DeclaringStruct != structType)) return context.Trace(MessageCode.ConstraintNotSatisfied, "Memberwise can only be applied to struct instances of the same type");

            Result<IValue> ApplyFuncToMemberPair(Port p) =>
                structs.Select(inst => inst![p.Identifier!.Value, false, context])
                       .BindEnumerable(fields => func.Call(fields.ToArray(), context));

            return structType.Fields.Select(ApplyFuncToMemberPair).MapEnumerable(resultFields => (IValue)new StructInstance(structType, resultFields));
        }
    }
}