namespace Element.AST
{
    public class InferIntrinsic : IIntrinsic, IFunction
    {
        public IType Type => FunctionType.Instance;
        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            throw new System.NotImplementedException();
        }
        public Port[] Inputs { get; } = {new Port("function", FunctionType.Annotation), new Port("instance", AnyConstraint.Annotation)};
        public TypeAnnotation Output { get; } = AnyConstraint.Annotation;

        public string Location => "infer";
    }
}