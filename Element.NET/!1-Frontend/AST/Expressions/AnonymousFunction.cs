namespace Element.AST
{
    public class AnonymousFunction : IFunctionWithBody
    {
        public AnonymousFunction(IScope callScope, object body, PortList inputs, Port output)
        {
            Body = body;
            _callScope = callScope;
            Output = output;
            Inputs =  inputs.List.ToArray();
        }

        private readonly IScope? _callScope;
        public IType Type => FunctionType.Instance;
        public Port[] Inputs { get; }
        public Port Output { get; }
        public IFunctionSignature GetDefinition(CompilationContext _) => this;

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            this.ResolveCall(arguments, _callScope, false, compilationContext);

        public object Body { get; }
    }
}