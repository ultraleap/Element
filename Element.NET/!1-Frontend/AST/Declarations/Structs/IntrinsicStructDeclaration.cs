namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class IntrinsicStructDeclaration : DeclaredStruct
    {
        protected override string IntrinsicQualifier => "intrinsic";

        internal override bool Validate(SourceContext sourceContext)
        {
            var success = base.Validate(sourceContext);
            if (DeclaredType != null)
            {
                sourceContext.LogError(19, $"Struct '{Identifier}' cannot have declared return type");
                success = false;
            }

            // Intrinsic structs implement constraint resolution and a callable constructor
            // They don't implement IScope, scope impl is still handled by DeclaredStruct
            success &= ImplementingIntrinsic<IType>(sourceContext) != null;
            success &= ImplementingIntrinsic<IFunction>(sourceContext) != null;

            return success;
        }

        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) => ImplementingIntrinsic<IConstraint>(compilationContext).MatchesConstraint(value, compilationContext);
        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) => ImplementingIntrinsic<IFunction>(compilationContext).Call(arguments, compilationContext);
    }
}