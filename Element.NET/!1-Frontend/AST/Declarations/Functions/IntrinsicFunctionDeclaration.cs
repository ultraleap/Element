namespace Element.AST
{
    public class IntrinsicFunctionDeclaration : FunctionDeclaration, IFunction
    {
        protected override string IntrinsicQualifier => "intrinsic";

        protected override bool AdditionalValidation(SourceContext sourceContext)
        {
            var success = ImplementingIntrinsic<IFunction>(sourceContext) != null;
            if (!(Body is Terminal))
            {
                sourceContext.LogError(20, $"Intrinsic function '{Location}' cannot have a body");
                success = false;
            }

            return success;
        }

        IValue IFunction.Call(IValue[] arguments, CompilationContext compilationContext) => ImplementingIntrinsic<IFunction>(compilationContext)?.Call(arguments, compilationContext) ?? CompilationError.Instance;
    }
}