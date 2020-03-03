using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IExpressionListStart {}

    public interface ISubExpression
    {
        IValue ResolveSubExpression(IValue previous, IScope resolutionScope, CompilationContext compilationContext);
    }

    [WhitespaceSurrounded, MultiLine]
    // ReSharper disable once ClassNeverInstantiated.Global
    public abstract class Expression
    {
        public abstract IValue ResolveExpression(IScope scope, CompilationContext compilationContext);
    }

    public class ExpressionChain : Expression
    {
        // ReSharper disable UnusedAutoPropertyAccessor.Local
        [field: Term] private IExpressionListStart LitOrId { get; set; }
        [field: Optional] private List<ISubExpression>? Expressions { get; set; }
        // ReSharper restore UnusedAutoPropertyAccessor.Local

        public override string ToString() => $"{LitOrId}{(Expressions != null ? string.Concat(Expressions) : string.Empty)}";

        public override IValue ResolveExpression(IScope scope, CompilationContext compilationContext)
        {
            var previous = LitOrId switch
            {
                // If the start of the list is an identifier, find the value that it identifies
                Identifier id => scope[id, compilationContext] is {} v
                    ? v
                    : compilationContext.LogError(7, $"Couldn't find '{id}' in local or outer scope"),
                Literal lit => lit,
                _ => throw new InternalCompilerException("Trying to compile expression that doesn't start with literal or identifier - should be impossible")
            };

            // Early out if something failed above
            if (previous is CompilationErr err) return err;

            compilationContext.Push(new TraceSite(previous.ToString(), null, 0, 0));

            previous = Expressions?.Aggregate(previous, (current, expr) => expr.ResolveSubExpression(current, scope, compilationContext)) ?? previous;

            previous = previous.ResolveNullaryFunction(compilationContext);

            compilationContext.Pop();

            return previous;
        }
    }

    public class AnonymousFunction : ICompilableFunction
    {

        public AnonymousFunction(IScope scope, object body, PortList ports)
        {
            Scope = scope;
            _body = body;
            Inputs =  ports?.List.ToArray() ?? Array.Empty<Port>();
        }

        private bool hasRecursed;
        private readonly object _body;
        public IType Type { get; }
        public IScope? Scope { get; set; }
        private IScope? CaptureScope { get; set; }
        public Port[]? Inputs { get; }
        public Type Output { get; }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            if (hasRecursed)
            {
                return compilationContext.LogError(11, "Recursion is disallowed");
            }

            if (arguments?.Length > 0)
            {
                // If there are any arguments we need to interject a capture scope to store them
                // The capture scope will be indexed before the parent scope when indexing a declared scope
                // Thus the order of indexing for an item becomes "Child -> Captures -> Parent"
                CaptureScope ??= new ArgumentCaptureScope(Scope, arguments.Select((arg, index) => (Inputs[index].Identifier, arg)));
            }

            hasRecursed = true;

            var callScope = CaptureScope ?? Scope;
            var result = arguments.ValidateArguments(Inputs, callScope, compilationContext)
                       ? Compile(callScope, compilationContext)
                       : CompilationErr.Instance;

            CaptureScope = null;
            hasRecursed = false;

            return result;
        }

        public IValue Compile(IScope scope, CompilationContext compilationContext) =>
            scope.CompileFunction(_body, compilationContext);
    }

    public class Lambda : Expression
    {
        [Literal("_")] public Unnamed _;
        [Term] public PortList _portList;
        [Alternative(typeof(Binding), typeof(Scope)), WhitespaceSurrounded, MultiLine] protected object Body;

        public override IValue ResolveExpression(IScope scope, CompilationContext compilationContext)
        {
            return new AnonymousFunction(scope, Body, _portList);
        }
    }
}