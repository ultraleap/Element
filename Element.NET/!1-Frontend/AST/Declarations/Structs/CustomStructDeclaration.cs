using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class CustomStructDeclaration : StructDeclaration
    {
        protected override string IntrinsicQualifier => string.Empty;

        protected override void AdditionalValidation(ResultBuilder builder)
        {
            if (DeclaredType != null)
            {
                builder.Append(MessageCode.StructCannotHaveReturnType, $"Struct '{Identifier}' cannot have declared return type");
            }

            if (!HasDeclaredInputs)
            {
                builder.Append(MessageCode.MissingPorts, $"Non intrinsic '{Location}' must have ports");
            }
        }

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) => new StructInstance(this, arguments);

        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => IsInstanceOfStruct(value, context);
        public override IReadOnlyList<Port> Fields => DeclaredInputs;
        public override Result<ISerializableValue> DefaultValue(CompilationContext context) =>
            Fields.Select(field => field.ResolveConstraint(context)
                                        .Bind(constraint => constraint is IType type
                                                                ? type.DefaultValue(context)
                                                                : context.Trace(MessageCode.TypeError, $"'{field}' is not a type - only types can produce a default value")))
                  .MapEnumerable(defaults => (ISerializableValue)new StructInstance(this, defaults.ToArray()));
    }
}