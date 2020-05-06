using System;
using System.Linq;

namespace Element.AST
{
    public sealed class ForIntrinsic : IntrinsicFunction
    {
        public ForIntrinsic()
            : base("for",
                   new[]
                   {
                       new Port(_initialIdentifier, AnyConstraint.Instance),
                       new Port(_conditionIdentifier, PredicateFunctionConstraint.Instance),
                       new Port(_bodyIdentifier, FunctionType.Instance)
                   }, Port.ReturnPort(AnyConstraint.Instance))
        { }

        private static readonly Identifier _initialIdentifier = new Identifier("initial");
        private static readonly Identifier _conditionIdentifier = new Identifier("condition");
        private static readonly Identifier _bodyIdentifier = new Identifier("body");

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            var initial = arguments[0];
            if (!initial.TrySerialize(out Element.Expression[] initialSerialized, compilationContext))
            {
                return CompilationError.Instance;
            }
            
            var condition = arguments[1] as IFunctionSignature;
            var body = arguments[2] as IFunctionSignature;
            var group = new Loop(initialSerialized,
                state => condition.ResolveCall(new[]{initial.Deserialize(state, compilationContext)}, false, compilationContext) as Element.Expression ?? CompilationError.Instance,
                state => body.ResolveCall(new[]{initial.Deserialize(state, compilationContext)}, false, compilationContext).Serialize(compilationContext));
            return initial.Deserialize(Enumerable.Range(0, group.Size).Select(i => new ExpressionGroupElement(group, i)), compilationContext);
        }
    }
}