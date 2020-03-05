using System;

namespace Element.AST
{
    public class AnonymousFunction : ICompilableFunction
    {
        public AnonymousFunction(IScope callScope, object body, PortList ports, Type output)
        {
            _body = body;
            _callScope = callScope;
            Output = output;
            Inputs =  ports?.List.ToArray() ?? Array.Empty<Port>();
        }

        private readonly object _body;
        private readonly IScope? _callScope;
        public IType Type => FunctionType.Instance;
        public Port[]? Inputs { get; }
        public Type Output { get; }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            this.ApplyArguments(arguments, Inputs, _body, _callScope, compilationContext);

        public IValue Compile(IScope scope, CompilationContext compilationContext) =>
            scope.CompileFunction(_body, compilationContext);

        public bool IsBeingCompiled { get; set; }
    }
}