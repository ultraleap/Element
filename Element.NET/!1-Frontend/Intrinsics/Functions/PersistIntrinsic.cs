namespace Element.AST
{
    public class PersistIntrinsic : IIntrinsic, IFunction
    {
        public IType Type => FunctionType.Instance;
        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            throw new System.NotImplementedException();
        }
        public Port[] Inputs { get; } = {new Port("initial", AnyConstraint.Annotation), new Port("body", FunctionType.Annotation)};
        public TypeAnnotation Output { get; } = AnyConstraint.Annotation;

        public string Location => "persist";
    }
}