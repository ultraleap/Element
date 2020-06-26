using System.Collections.Generic;

namespace Element.AST
{
    public sealed class IntrinsicFunctionDeclaration : FunctionDeclaration
    {
        protected override string IntrinsicQualifier => "intrinsic";

        protected override void AdditionalValidation(ResultBuilder builder)
        {
            if (!(Body is Terminal))
            {
                builder.Append(MessageCode.IntrinsicCannotHaveBody, $"Intrinsic function '{Location}' cannot have a body");
            }
            // TODO: Check declared inputs/outputs match the compiler-defined ones
            builder.Append(IntrinsicCache.Get<IntrinsicFunction>(Identifier, builder.Trace));
        }

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) => Implementation.Call(arguments, context);
        public override IReadOnlyList<Port> Inputs => Implementation.Inputs;
        public override Port Output => Implementation.Output;

        private IntrinsicFunction Implementation
        {
            get
            {
                var intrinsic = IntrinsicCache.Get<IntrinsicFunction>(Identifier, null);
                if (intrinsic.IsSuccess) return intrinsic.ResultOr(default!); // Guaranteed to return result as we checked first
                throw new InternalCompilerException($"No intrinsic '{Identifier.Value}' - this exception can only occur if validation is skipped");
            }
        }
    }
}