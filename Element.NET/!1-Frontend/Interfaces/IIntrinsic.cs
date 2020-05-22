namespace Element.AST
{
    public interface IIntrinsic : IValue
    {
        string Location { get; }
    }
    
    public interface IIntrinsicType : IIntrinsic, IFunction, IType { }
    
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

        public string Location { get; }
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);
        Port[] IFunctionSignature.Inputs => _inputs;
        Port IFunctionSignature.Output => _output;
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) =>
            compilationContext.GetIntrinsicsDeclaration<IntrinsicFunctionDeclaration>(this);
    }
    
    public static class IntrinsicHelpers
    {
        public static IntrinsicStructDeclaration GetDeclaration(this IIntrinsicType type, CompilationContext compilationContext) =>
            compilationContext.GetIntrinsicsDeclaration<IntrinsicStructDeclaration>(type);
    }
}