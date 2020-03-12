namespace Element.AST
{
    public class AnonymousFunction : ICompilableFunction
    {
        public AnonymousFunction(IScope callScope, object body, PortList ports, TypeAnnotation? output)
        {
            _body = body;
            _callScope = callScope;
            Output = output;
            Inputs =  ports.List.ToArray();
        }

        private readonly object _body;
        private readonly IScope? _callScope;
        public IType Type => FunctionType.Instance;
        public Port[] Inputs { get; }
        public TypeAnnotation? Output { get; }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            this.ApplyArguments(arguments, Inputs, Output, _body, _callScope, compilationContext);

        public IValue Compile(IScope scope, CompilationContext compilationContext) =>
            scope.CompileFunction(_body, compilationContext);

        public ICompilableFunction Definition => this;
    }
}