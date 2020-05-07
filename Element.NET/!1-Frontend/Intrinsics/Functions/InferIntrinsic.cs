using System.Linq;

namespace Element.AST
{
    public class InferIntrinsic : IntrinsicFunction
    {
        public InferIntrinsic()
            : base("infer",
                   new[]
                   {
                       new Port("function", FunctionType.Instance),
                       new Port("instance", AnyConstraint.Instance)
                   }, Port.ReturnPort(AnyConstraint.Instance))
        { }

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            var fn = (IFunctionSignature) arguments[0];
            if (!(arguments[1] is IIndexable instance)) return compilationContext.LogError(8, "Instance passed to infer must be an indexable value");
            var fnArgs = fn.Inputs.Select(p => instance[p.Identifier.Value, false, compilationContext]).ToArray();
            return fn.ResolveCall(fnArgs, false, compilationContext);
        }
    }
}