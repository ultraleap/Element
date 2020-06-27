using System;
using System.Collections.Generic;

namespace Element.AST
{
    public abstract class StructDeclaration : Declaration, IScope, IFunctionSignature
    {
        protected override string Qualifier { get; } = "struct";
        protected override Type[] BodyAlternatives { get; } = {typeof(Block), typeof(Terminal)};
        protected override Identifier[] ScopeIdentifierBlacklist => new[]{Identifier};

        public override string ToString() => $"{Location}:Struct";

        public IReadOnlyList<Port> Fields => DeclaredInputs;
        public Result<IValue> Lookup(Identifier id, CompilationContext context) => (Child ?? Parent).Lookup(id, context);
        public override IReadOnlyList<IValue> Members => Child?.Members ?? Array.Empty<IValue>();

        IReadOnlyList<Port> IFunctionSignature.Inputs => DeclaredInputs;
        Port IFunctionSignature.Output => Port.ReturnPort(this);
        
        public Result<bool> IsInstanceOfStruct(IValue value, CompilationContext context) => value.FullyResolveValue(context).Map(v => v is StructInstance instance && instance.DeclaringStruct == this);
        
        public Result<IValue> ResolveInstanceFunction(Identifier instanceFunctionIdentifier, IValue instanceBeingIndexed, CompilationContext context) =>
            Index(instanceFunctionIdentifier, context)
                .Bind(v => v switch
                {
                    FunctionSignatureDeclaration instanceFunction when instanceFunction.IsNullary() => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"Constant '{instanceFunction.Location}' cannot be accessed by indexing an instance"),
                    IFunctionSignature function => function.Inputs[0].ResolveConstraint(this, context).Bind(constraint => constraint switch
                    {
                        {} when constraint == this => function.PartiallyApply(new[] {instanceBeingIndexed}, context),
                        _ => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"Found function '{function}' <{function.Inputs[0]}> must be of type <:{Identifier}> to be used as an instance function")
                    }),
                    Declaration notInstanceFunction => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"'{notInstanceFunction.Location}' is not a function"),
                    {} notInstanceFunction => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"'{notInstanceFunction}' found by indexing '{instanceBeingIndexed}' is not a function"),
                    _ => context.Trace(MessageCode.IdentifierNotFound, $"Couldn't find any member or instance function '{instanceFunctionIdentifier}' for '{instanceBeingIndexed}' of type <{this}>")
                });
    }
}