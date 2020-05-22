using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;

namespace Element.AST
{
    public sealed class ForIntrinsic : IntrinsicFunction
    {
        public ForIntrinsic()
            : base("for",
                   new[]
                   {
                       new Port(_initialIdentifier, SerializableConstraint.Instance),
                       new Port(_conditionIdentifier, PredicateFunctionConstraint.Instance),
                       new Port(_bodyIdentifier, FunctionConstraint.Instance)
                   }, Port.ReturnPort(AnyConstraint.Instance))
        { }

        private static readonly Identifier _initialIdentifier = new Identifier("initial");
        private static readonly Identifier _conditionIdentifier = new Identifier("condition");
        private static readonly Identifier _bodyIdentifier = new Identifier("body");

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            if (!(arguments[0] is ISerializableValue initial))
            {
                return CompilationError.Instance;
            }

            var initialSerialized = initial.Serialize(compilationContext);
            var condition = (IFunctionSignature) arguments[1];
            var body = (IFunctionSignature) arguments[2];

            Element.Expression Condition(ReadOnlyCollection<State> state) => condition.ResolveCall(new[] {initial.Deserialize(state, compilationContext)}, false, compilationContext) as Element.Expression ?? CompilationError.Instance;

            IEnumerable<Element.Expression> Body(ReadOnlyCollection<State> state) => body.ResolveCall(new[] {initial.Deserialize(state, compilationContext)}, false, compilationContext).Serialize(compilationContext);

            var loop = new Loop(initialSerialized, Condition, Body);
            return initial.Deserialize(Enumerable.Range(0, loop.Size).Select(i => new ExpressionGroupElement(loop, i)), compilationContext);
        }
    }
}