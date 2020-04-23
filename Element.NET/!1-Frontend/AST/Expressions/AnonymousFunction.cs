namespace Element.AST
{
    public class AnonymousFunction : IFunctionWithBody
    {
        public AnonymousFunction(object body, PortList inputs, Port output)
        {
            Body = body;
            Output = output;
            Inputs =  inputs.List.ToArray();
        }

        public IType Type => FunctionType.Instance;
        public Port[] Inputs { get; }
        public Port Output { get; }
        public IFunctionSignature GetDefinition(CompilationContext _) => this;

        public object Body { get; }
    }
}