namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class IntrinsicStructDeclaration : StructDeclaration
    {
        protected override string IntrinsicQualifier => "intrinsic";

        protected override bool AdditionalValidation(SourceContext sourceContext)
        {
            var success = true;
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

        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) => ImplementingIntrinsic<IConstraint>(compilationContext)?.MatchesConstraint(value, compilationContext) ?? false;
        public override ISerializableValue DefaultValue(CompilationContext context) => ImplementingIntrinsic<IType>(context)?.DefaultValue(context) ?? CompilationError.Instance;
        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) => ImplementingIntrinsic<IFunction>(compilationContext)?.Call(arguments, compilationContext) ?? CompilationError.Instance;
    }
}