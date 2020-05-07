using System.Linq;

namespace Element.AST
{
    public class MemberwiseIntrinsic : IntrinsicFunction
    {
        public MemberwiseIntrinsic()
            : base("memberwise",
                   new[]
                   {
                       new Port("function", FunctionType.Instance),
                       Port.VariadicPort
                   }, Port.ReturnPort(AnyConstraint.Instance))
        { }

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            var funcArgs = arguments.Skip(1).ToArray();
            if (funcArgs.Any(v => v is IFunctionSignature fn && fn.Inputs.Length > 0))
            {
                return compilationContext.LogError(14, "Memberwise cannot operate on member functions with inputs");
            }


            var type = funcArgs[0].Type;
            if (funcArgs.Any(v => v.Type != type))
            {
                return compilationContext.LogError(14, "Arguments to memberwise must be homogeneous (all of same type)");
            }

            var func = arguments[0] as IFunctionSignature;
            if (type == NumType.Instance)
            {
                return func.ResolveCall(funcArgs, false, compilationContext);
            }

            // TODO: Needs to return anonymous scope, not a struct instance! This currently only works when the function outputs the same type as the inputs, e.g. (Vec3, Vec3) -> Vec3
            if (type is DeclaredStruct declaredStruct)
            {
                return declaredStruct.CreateInstance(((IFunctionSignature)declaredStruct).Inputs
                                                               .Select(p => func.ResolveCall(funcArgs.Cast<StructInstance>()
                                                                                                     .Select(inst => inst[p.Identifier.Value, false, compilationContext])
                                                                                                     .ToArray(), false, compilationContext))
                                                               .ToArray());
            }

            return compilationContext.LogError(14, "Arguments to memberwise must be constants or struct instances");
        }
    }
}