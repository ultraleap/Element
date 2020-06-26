using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;

namespace Element.AST
{
    public sealed class ForIntrinsic : IntrinsicFunction
    {
        private ForIntrinsic()
        {
            Identifier = new Identifier("for");
            Inputs = new[]
            {
                new Port(_initialIdentifier, SerializableConstraint.Instance),
                new Port(_conditionIdentifier, PredicateFunctionConstraint.Instance),
                new Port(_bodyIdentifier, FunctionConstraint.Instance)
            };
            Output = Port.ReturnPort(AnyConstraint.Instance);
        }

        public static ForIntrinsic Instance { get; } = new ForIntrinsic();

        private static readonly Identifier _initialIdentifier = new Identifier("initial");
        private static readonly Identifier _conditionIdentifier = new Identifier("condition");
        private static readonly Identifier _bodyIdentifier = new Identifier("body");

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context)
        {
            var initial = (ISerializableValue) arguments[0];
            var condition = (IFunction) arguments[1];
            var body = (IFunction) arguments[2];

            Result<Element.Expression> Condition(ReadOnlyCollection<State> state) =>
                initial.Deserialize(state, context)
                       .Bind(value => condition.Call(new[] {value}, context))
                       .Cast<Element.Expression>(context);
            
            Result<IEnumerable<Element.Expression>> Body(ReadOnlyCollection<State> state) =>
                initial.Deserialize(state, context)
                       .Bind(value => body.Call(new[] {value}, context))
                       .Bind(result => result.Serialize(context))
                       .Cast<IEnumerable<Element.Expression>>(context);

            return initial.Serialize(context)
                          .Bind(initialSerialized => Loop.TryCreate(initialSerialized, Condition, Body))
                          .Bind(loop => initial.Deserialize(Enumerable.Range(0, loop.Size).Select(i => new ExpressionGroupElement(loop, i)), context))
                          .Cast<IValue>(context);
        }

        public override IReadOnlyList<Port> Inputs { get; }
        public override Port Output { get; }
        public override Identifier Identifier { get; }
    }
}