namespace Element.AST
{
    public class FunctionDeclaration : DeclaredFunction, IFunctionWithBody
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override Identifier[] ScopeIdentifierWhitelist { get; } = {Parser.ReturnIdentifier};

        internal override bool Validate(SourceContext sourceContext)
        {
            var success = base.Validate(sourceContext);
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