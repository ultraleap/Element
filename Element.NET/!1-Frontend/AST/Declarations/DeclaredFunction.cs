using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class DeclaredFunction : DeclaredItem<IntrinsicFunction>, IValue, IFunction
    {
        protected override string Qualifier { get; } = string.Empty; // Functions don't have a qualifier
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
        protected override List<Identifier> ScopeIdentifierWhitelist { get; } = new List<Identifier> {Parser.ReturnIdentifier};
        IType IValue.Type => FunctionType.Instance;
        Port[] IFunction.Inputs => DeclaredInputs;
        Type IFunction.Output => DeclaredType;

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = ValidateIntrinsic(compilationContext) && ValidateScopeBody(compilationContext);
            if (!IsIntrinsic && Body is Terminal)
            {
                compilationContext.LogError(21, $"Non intrinsic function '{Location}' must have a body");
                success = false;
            }

            return success;
        }

        private sealed class CallScope : ScopeBase
        {
            public CallScope(IScope parent, IEnumerable<(Identifier Identifier, IValue Value)> members)
            {
                Parent = parent;
                SetRange(members);
            }

            public override string Location => $"Call to <{Parent.Location}>";
        }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext)
        {
            IValue CompileFunction(DeclaredFunction function, IScope parentScope) =>
                function.Body switch
                {
                    // If a function is a binding (has expression body) we just compile the single expression
                    Binding binding => binding.Expression.ResolveExpression(parentScope, compilationContext),

                    // If a function has a scope body we find the return value
                    Scope scope => scope[Parser.ReturnIdentifier, compilationContext] switch
                    {
                        // If the return value is a function, compile it
                        DeclaredFunction returnFunction => CompileFunction(returnFunction, parentScope),
                        null => compilationContext.LogError(7, $"'{Parser.ReturnIdentifier}' not found in function scope"),
                        // TODO: Add support for returning other items as values - structs and namespaces
                        var nyi => throw new NotImplementedException(nyi.ToString())
                    },
                    _ => CompilationErr.Instance
                };

            if (IsIntrinsic) return GetImplementingIntrinsic(compilationContext)?.Call(arguments, compilationContext);

            var argumentsValid = arguments.ValidateArgumentCount(DeclaredInputs?.Length ?? 0, compilationContext)
                                 && arguments.ValidateArgumentConstraints(DeclaredInputs, Body as IScope ?? Parent, compilationContext);
            return argumentsValid
                ? CompileFunction(this, arguments?.Length > 0
                    // If we have any arguments for this function, push a call scope
                    // else we  parent scope for the function is the declaration's parent
                    ? (IScope) new CallScope(Parent, arguments.Select((arg, index) => (DeclaredInputs[index].Identifier, arg)))
                    : Parent)
                : CompilationErr.Instance;
        }

        private class InstanceFunction : IFunction, IValue
        {
            public InstanceFunction(IValue value, IFunction function)
            {
                _surrogate = function;
                _argument = value;
                Inputs = _surrogate.Inputs.Skip(1).ToArray();
            }

            private readonly IFunction _surrogate;
            private readonly IValue _argument;

            public Port[] Inputs { get; }
            public Type Output => _surrogate.Output;
            IType IValue.Type => FunctionType.Instance;

            public IValue Call(IValue[] arguments, CompilationContext compilationContext)
            {
                // TODO: Argument validation - validation will be done by the surrogate call currently

                var result = _surrogate.Call(arguments.Prepend(_argument).ToArray(), compilationContext);
                return result.ResolveNullaryFunction(compilationContext);
            }
        }

        public static IValue ResolveAsInstanceFunction(Identifier instanceFunctionIdentifier, IValue instanceBeingIndexed, DeclaredStruct type, CompilationContext compilationContext) =>
            type[instanceFunctionIdentifier, compilationContext] switch
            {
                DeclaredFunction instanceFunction when instanceFunction.IsNullary() => compilationContext.LogError(22, $"Constant '{instanceFunction.Location}' cannot be accessed by indexing an instance"),
                IFunction instanceFunction when instanceFunction.Inputs[0].Type.ResolveConstraint(type, compilationContext) == type => new InstanceFunction(instanceBeingIndexed, instanceFunction),
                DeclaredItem notInstanceFunction => compilationContext.LogError(22, $"'{notInstanceFunction.Location}' is not a function"),
                {} notInstanceFunction => compilationContext.LogError(22, $"'{notInstanceFunction}' found by indexing '{instanceBeingIndexed}' is not a function"),
                _ => compilationContext.LogError(7, $"Couldn't find any member or instance function '{instanceFunctionIdentifier}' for '{instanceBeingIndexed}' of type <{type}>")
            };
    }
}