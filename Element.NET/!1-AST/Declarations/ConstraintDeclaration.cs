using System;

namespace Element.AST
{
    public class IntrinsicConstraintDeclaration : Declaration
    {
        protected override string IntrinsicQualifier { get; } = "intrinsic";
        protected override string Qualifier { get; } = "constraint ";
        protected override Type[] BodyAlternatives { get; } = {typeof(Nothing)};
        protected override Result<IValue> ResolveImpl(IScope scope, Context context) =>
            IntrinsicImplementationCache.Get<IIntrinsicConstraintImplementation>(Identifier, context)
                          .Map(intrinsic => (IValue)new IntrinsicConstraint(intrinsic));

        protected override void ValidateDeclaration(ResultBuilder builder, Context context)
        {
            builder.Append(IntrinsicImplementationCache.Get<IIntrinsicConstraintImplementation>(Identifier, builder.Context));
            if (PortList != null) builder.Append(EleMessageCode.IntrinsicConstraintCannotSpecifyFunctionSignature, "Intrinsic constraint cannot specify ports");
            if (ReturnConstraint != null) builder.Append(EleMessageCode.IntrinsicConstraintCannotSpecifyFunctionSignature, "Intrinsic constraint cannot specify a return constraint");
        }
    }
    
    public class CustomConstraintDeclaration : Declaration
    {
        protected override string IntrinsicQualifier { get; } = string.Empty;

        protected override string Qualifier { get; } = "constraint ";
        protected override Type[] BodyAlternatives { get; } = {typeof(Nothing)};
        protected override Result<IValue> ResolveImpl(IScope scope, Context context) =>
            PortList.ResolveInputConstraints(scope, context, false, false)
                    .Accumulate(() => ReturnConstraint.ResolveReturnConstraint(scope, context))
                    .Map(t =>
                    {
                        var (inputPorts, returnConstraint) = t;
                        return (IValue)new FunctionConstraint(inputPorts, returnConstraint);
                    });

        protected override void ValidateDeclaration(ResultBuilder builder, Context context)
        {
            if (PortList == null || PortList.Ports.List.Count == 0)
            {
                builder.Append(EleMessageCode.MissingPorts, $"Non-intrinsic constraint '{this}' must have a port list");
            }
            PortList?.Validate(builder, context);
            ReturnConstraint?.Validate(builder, context);
        }
    }
}