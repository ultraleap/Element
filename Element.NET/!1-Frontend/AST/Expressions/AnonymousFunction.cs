namespace Element.AST
{
    public class AnonymousFunction : IFunctionWithBody
    {
        public AnonymousFunction(IScope parent, object body, PortList inputs, Port output)
        {
            Body = body;
            Parent = parent;
            Output = output;
            Inputs =  inputs.Ports.List.ToArray();
        }

        public IType Type => FunctionType.Instance;
        public Port[] Inputs { get; }
        public Port Output { get; }
        public IFunctionSignature GetDefinition(CompilationContext _) => this;

        public IScope Parent { get; }
        public object Body { get; }
    }
}