namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class StructDeclaration : DeclaredStruct
    {
        protected override string IntrinsicQualifier => string.Empty;

        internal override bool Validate(SourceContext sourceContext)
        {
            var success = base.Validate(sourceContext);

            if (DeclaredType != null)
            {
                sourceContext.LogError(19, $"Struct '{Identifier}' cannot have declared return type");
                success = false;
            }

            if (!HasDeclaredInputs)
            {
                sourceContext.LogError(13, $"Non intrinsic '{Location}' must have ports");
                success = false;
            }

            return success;
        }

        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == this;

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) => CreateInstance(arguments);
    }
}