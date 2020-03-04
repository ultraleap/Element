using System;

namespace Element.AST
{
    public class AnonymousFunction : ICompilableFunction
    {
        public AnonymousFunction(IScope parentScope, object body, PortList ports, Type output)
        {
            _parentScope = parentScope;
            _body = body;
            Output = output;
            Inputs =  ports?.List.ToArray() ?? Array.Empty<Port>();
        }

        private readonly object _body;
        public IType Type => FunctionType.Instance;
        private readonly IScope? _parentScope;
        public Port[]? Inputs { get; }
        public Type Output { get; }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            this.ResolveCall(arguments, Inputs, _parentScope, compilationContext);

        public IValue Compile(IScope scope, CompilationContext compilationContext) =>
            scope.CompileFunction(_body, compilationContext);
    }
}