using System;

namespace Element.AST
{
    public abstract class ConstraintDeclaration : Declaration
    {
        protected override string Qualifier { get; } = "constraint";
        protected override Type[] BodyAlternatives { get; } = {typeof(Terminal)};
    }
    
    public class IntrinsicConstraintDeclaration : ConstraintDeclaration
    {
        protected override string IntrinsicQualifier { get; } = "intrinsic";
        protected override Result<IValue> ResolveImpl(IScope scope, CompilationContext context) =>
            IntrinsicCache.Get<IntrinsicConstraintImplementation>(Identifier, context)
                          .Map(intrinsic => (IValue)new IntrinsicConstraint(intrinsic, context.CurrentDeclarationLocation));

        protected override void AdditionalValidation(ResultBuilder builder, CompilationContext context)
        {
            builder.Append(IntrinsicCache.Get<IntrinsicConstraintImplementation>(Identifier, builder.Trace));
            if (PortList != null) builder.Append(MessageCode.IntrinsicConstraintCannotSpecifyFunctionSignature, "Intrinsic constraint cannot specify ports");
            if (ReturnConstraint != null) builder.Append(MessageCode.IntrinsicConstraintCannotSpecifyFunctionSignature, "Intrinsic constraint cannot specify a return constraint");
        }
    }
    
    public class CustomConstraintDeclaration : ConstraintDeclaration
    {
        protected override string IntrinsicQualifier { get; } = string.Empty;

        protected override Result<IValue> ResolveImpl(IScope scope, CompilationContext context) =>
            PortList.ResolveInputConstraints(scope, context, false, false)
                    .Accumulate(() => ReturnConstraint.ResolveReturnConstraint(scope, context))
                    .Map(t =>
                    {
                        var (inputPorts, returnConstraint) = t;
                        return (IValue)new FunctionConstraint(inputPorts, returnConstraint, context.CurrentDeclarationLocation);
                    });

        protected override void AdditionalValidation(ResultBuilder builder, CompilationContext context)
        {
            if (PortList == null || PortList.Ports.List.Count == 0)
            {
                builder.Append(MessageCode.MissingPorts, $"Non-intrinsic constraint '{context.CurrentDeclarationLocation}' must have a port list");
            }
        }
    }
}