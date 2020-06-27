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
        public override Result<IValue> DefaultValue(CompilationContext context) =>
            Fields.Select(field => field.ResolveConstraint(context).Bind(v => v.DefaultValue(context)))
                  .MapEnumerable(defaults => (IValue)new StructInstance(this, defaults.ToArray()));
    }
}