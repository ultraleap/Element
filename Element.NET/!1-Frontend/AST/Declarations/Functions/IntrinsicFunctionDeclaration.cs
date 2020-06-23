using System.Collections.Generic;

namespace Element.AST
{
    public sealed class IntrinsicFunctionDeclaration : FunctionDeclaration, IFunction
    {
        protected override string IntrinsicQualifier => "intrinsic";

        protected override void AdditionalValidation(ResultBuilder builder)
        {
            if (!(Body is Terminal))
            {
                builder.Append(MessageCode.IntrinsicCannotHaveBody, $"Intrinsic function '{Location}' cannot have a body");
            }
            // TODO: Check declared inputs/outputs match the compiler-defined ones
        }

        public Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) => _implementation.Call(arguments, context); // TODO: All the other stuff (from ResolveCall)
        public override Port[] Inputs { get; }
        public override Port Output { get; }

        private IntrinsicFunction _implementation => Identifier.Value switch
        {
            "Num" => NumType.Instance,
            _ =>
        };
    }
}