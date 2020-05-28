namespace Element.AST
{
    public class CustomFunctionDeclaration : FunctionDeclaration, IFunctionWithBody
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override Identifier[] ScopeIdentifierWhitelist { get; } = {Parser.ReturnIdentifier};

        protected override bool AdditionalValidation(SourceContext sourceContext)
        {
            var success = true;
            if (Body is Terminal)
            {
                sourceContext.LogError(21, $"Non intrinsic function '{Location}' must have a body");
                success = false;
            }

            return success;
        }

        object IFunctionWithBody.Body => Body;
    }
}