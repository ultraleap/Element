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

        private bool hasRecursed;
        private readonly object _body;
        public IType Type => FunctionType.Instance;
        private readonly IScope? _parentScope;
        private IScope? _captureScope;
        public Port[]? Inputs { get; }
        public Type Output { get; }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            this.ResolveCall(arguments, ref _captureScope, ref hasRecursed, Inputs, null, _parentScope, compilationContext);

        public IValue Compile(IScope scope, CompilationContext compilationContext) =>
            scope.CompileFunction(_body, compilationContext);

        public ICompilableFunction Clone(CompilationContext compilationContext)
        {
            throw new NotImplementedException();
        }
    }
}