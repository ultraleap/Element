using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;

namespace Element.AST
{
    public sealed class For : IntrinsicValue, IIntrinsicFunctionImplementation
    {
        private For()
        {
            _identifier = new Identifier("for");
        }
        
        public static For Instance { get; } = new For();
        protected override Identifier _identifier { get; }
        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context)
        {
            var initial = arguments[0];
            var condition = arguments[1];
            var body = arguments[2];

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
    }
}