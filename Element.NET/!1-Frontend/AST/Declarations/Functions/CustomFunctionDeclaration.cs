namespace Element.AST
{
    public class CustomFunctionDeclaration : FunctionDeclaration, IFunctionWithBody
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override Identifier[] ScopeIdentifierWhitelist { get; } = {Parser.ReturnIdentifier};
        
        public override Port[] Inputs => DeclaredInputs;
        public override Port Output => DeclaredOutput;

        protected override bool AdditionalValidation(SourceContext sourceContext)
        {
            var success = true;
            if (Body is Terminal)
            {
                sourceContext.Flush(21, $"Non intrinsic function '{Location}' must have a body");
                success = false;
            }

            return success;
        }

        object IFunctionWithBody.Body => Body;
        
        /*// Local function since it can call into itself recursively
        static IValue ResolveFunctionBody(object body, IScope scope, CompilationContext compilationContext) =>
            body switch
            {
                // If a function has expression body we just compile the single expression using the call scope
                ExpressionBody exprBody => exprBody.Expression.ResolveExpression(scope, compilationContext),
                // If a function has a scope body we need to find the return identifier
                IScope scopeBody => scopeBody[Parser.ReturnIdentifier, false] switch
                {
                    IFunctionWithBody nullaryReturn when nullaryReturn.IsNullary() => ResolveFunctionBody(
                        nullaryReturn.Body, scope, compilationContext),
                    IFunctionWithBody functionReturn => functionReturn,
                    null => compilationContext.Flush(
                        7, $"'{Parser.ReturnIdentifier}' not found in function scope"),
                    var nyi => throw new NotImplementedException(nyi.ToString())
                },
                _ => throw new InternalCompilerException("Function body type unrecognized")
            };*/
    }
}