using System.Linq;

namespace Element.AST
{
    public class MemberwiseIntrinsic : IntrinsicFunction
    {
        public MemberwiseIntrinsic()
            : base("memberwise",
                   new[]
                   {
                       new Port("function", FunctionConstraint.Instance),
                       Port.VariadicPort
                   }, Port.ReturnPort(AnyConstraint.Instance))
        { }

        public override Result<IValue> Call(IValue[] arguments, CompilationContext compilationContext)
        {
            var func = (IFunctionSignature)arguments[0];
            var structs = arguments.Skip(1).Select(arg => arg as StructInstance).ToArray();
            if (structs.Any(s => s == null)) return compilationContext.Flush(8, "Memberwise can only be applied to struct instances");
            if (structs.Length < 1) return compilationContext.Flush(8, "Memberwise requires at least 1 struct to apply the given function");
            var structType = structs[0]!.DeclaringStruct;
            if (structType.Fields.Any(f => !f.Identifier.HasValue)) return compilationContext.Flush(8, "Memberwise cannot be applied to structs with ");
            if (structs.Any(s => s!.DeclaringStruct != structType)) return compilationContext.Flush(8, "Memberwise can only be applied to struct instances of the same type");

            IValue ApplyFuncToMemberPair(Port p) => func.ResolveCall(structs.Select(inst => inst![p.Identifier!.Value, false, TODO]).ToArray(), false, compilationContext);
            return structType.CreateInstance(structType.Fields.Select(ApplyFuncToMemberPair).ToArray());
        }
    }
}