namespace Element.AST
{
    public abstract class IntrinsicFunction : IIntrinsic, IFunction
    {
        private readonly Port[] _inputs;
        private readonly Port _output;

        protected IntrinsicFunction(string location, Port[] inputs, Port output)
        {
            Location = location;
            _inputs = inputs;
            _output = output;
        }

        public IType Type => FunctionType.Instance;
        public string Location { get; }
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
        Port[] IFunctionSignature.Inputs => _inputs;
        Port IFunctionSignature.Output => _output;
        IFunctionSignature IFunctionSignature.GetDefinition(CompilationContext context) => context.GetIntrinsicsDeclaration<IntrinsicFunctionDeclaration>(this);
    }
}