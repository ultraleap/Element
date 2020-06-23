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

        public override Result<bool> MatchesConstraint(IValue value, CompilationContext compilationContext) => value is StructInstance instance && instance.DeclaringStruct == this;
        public override Result<Port[]> Fields => DeclaredInputs;

        public override Result<ISerializableValue> DefaultValue(CompilationContext compilationContext) =>
            Fields.Bind(fields => fields.Select(field => field.ResolveConstraint(compilationContext)
                                                              .Bind(constraint => constraint is IType type
                                                                                      ? type.DefaultValue(compilationContext)
                                                                                      : compilationContext.Trace(MessageCode.TypeError, $"'{field}' is not a type - only types can produce a default value")))
                                        .MapEnumerable(defaults => (ISerializableValue)CreateInstance(defaults.ToArray())));
        protected override Result<IValue> Construct(IEnumerable<IValue> arguments) => CreateInstance(arguments);
    }
}