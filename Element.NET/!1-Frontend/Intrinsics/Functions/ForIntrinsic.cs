using System;

namespace Element.AST
{
    public sealed class ForIntrinsic : IntrinsicFunction
    {
        public ForIntrinsic()
            : base("for",
                   new[]
                   {
                       new Port(_initialIdentifier, AnyConstraint.Instance),
                       new Port(_conditionIdentifier, FunctionType.Instance),
                       new Port(_bodyIdentifier, FunctionType.Instance)
                   }, Port.ReturnPort(AnyConstraint.Instance))
        { }

        private static readonly Identifier _initialIdentifier = new Identifier("initial");
        private static readonly Identifier _conditionIdentifier = new Identifier("condition");
        private static readonly Identifier _bodyIdentifier = new Identifier("body");

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            throw new NotImplementedException();
            /*if (this.CheckArguments(arguments, output, context) != null)
            {
                return CompilationError.Instance;
            }

            var initial = arguments[0];
            if (arguments.Any(a => a is IType))
            {
                return initial;
            }

            var initialSerialized = initial.Serialize(context);
            var condition = arguments[1];
            var body = arguments[2];
            var group = new Loop(initialSerialized,
                state => condition.Call(new[] {initial.Deserialize(state, context)}, context).AsExpression(context),
                state => body.Call(new[] {initial.Deserialize(state, context)}, context).Serialize(context));
            return initial.Deserialize(Enumerable.Range(0, group.Size).Select(i => new ExpressionGroupElement(group, i)), context);
            */
        }
    }
}