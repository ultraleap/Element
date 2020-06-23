using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;

namespace Element.AST
{
    public sealed class ForIntrinsic : IntrinsicFunction
    {
        public ForIntrinsic()
        {
            Identifier = new Identifier("For");
            Inputs = new[]
            {
                new Port(_initialIdentifier, SerializableConstraint.Instance),
                new Port(_conditionIdentifier, PredicateFunctionConstraint.Instance),
                new Port(_bodyIdentifier, FunctionConstraint.Instance)
            };
            Output = Port.ReturnPort(AnyConstraint.Instance);
        }

        private static readonly Identifier _initialIdentifier = new Identifier("initial");
        private static readonly Identifier _conditionIdentifier = new Identifier("condition");
        private static readonly Identifier _bodyIdentifier = new Identifier("body");

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context)
        {
            var initial = (ISerializableValue)arguments[0];
            var condition = (IFunction) arguments[1];
            var body = (IFunction) arguments[2];
            
            Element.Expression Condition(ReadOnlyCollection<State> state) => condition.Call(new[] {initial.Deserialize(state, context)}, context) as Element.Expression ?? CompilationError.Instance;
            IEnumerable<Element.Expression> Body(ReadOnlyCollection<State> state) => body.Call(new[] {initial.Deserialize(state, context)}, context).Serialize(context);
            
            return initial.Serialize(context)
                          .Bind(initialSerialized =>
                          {
                              var loop = new Loop(initialSerialized, Condition, Body);
                              return initial.Deserialize(Enumerable.Range(0, loop.Size).Select(i => new ExpressionGroupElement(loop, i)), context);
                          })
                          .Map(sv => (IValue) sv);
        }

        public override IReadOnlyList<Port> Inputs { get; }
        public override Port Output { get; }
        public override Identifier Identifier { get; }
    }