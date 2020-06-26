using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public abstract class StructDeclaration : Declaration, IScope, IFunction, IType
    {
        protected override string Qualifier { get; } = "struct";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope), typeof(Terminal)};
        protected override Identifier[] ScopeIdentifierBlacklist => new[]{Identifier};

        public override string ToString() => $"{Location}:Struct";

        public abstract IReadOnlyList<Port> Fields { get; }
        public Result<IValue> this[Identifier id, bool recurse, CompilationContext context] => (Child ?? Parent)[id, recurse, context];
        public int Count => Child?.Count ?? 0;
        public IEnumerator<IValue> GetEnumerator() => Child?.GetEnumerator() ?? Enumerable.Empty<IValue>().GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();


        IReadOnlyList<Port> IFunction.Inputs => DeclaredInputs;
        Port IFunction.Output => Port.ReturnPort(this);
        public abstract Result<ISerializableValue> DefaultValue(CompilationContext context);
        public abstract Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context);
        public abstract Result<bool> MatchesConstraint(IValue value, CompilationContext context);
        
        public Result<bool> IsInstanceOfStruct(IValue value, CompilationContext context) => value.FullyResolveValue(context).Map(v => v is StructInstance instance && instance.DeclaringStruct == this);
        
        public Result<IValue> ResolveInstanceFunction(Identifier instanceFunctionIdentifier, IValue instanceBeingIndexed, CompilationContext context) =>
            this[instanceFunctionIdentifier, false, context]
                .Bind(v => v switch
                {
                    FunctionDeclaration instanceFunction when instanceFunction.IsNullary() => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"Constant '{instanceFunction.Location}' cannot be accessed by indexing an instance"),
                    IFunction function => function.Inputs[0].ResolveConstraint(this, context).Bind(constraint => constraint switch
                    {
                        {} when constraint == this => function.PartiallyApply(new[] {instanceBeingIndexed}, context),
                        _ => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"Found function '{function}' <{function.Inputs[0]}> must be of type <:{Identifier}> to be used as an instance function")
                    }),
                    Declaration notInstanceFunction => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"'{notInstanceFunction.Location}' is not a function"),
                    {} notInstanceFunction => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"'{notInstanceFunction}' found by indexing '{instanceBeingIndexed}' is not a function"),
                    _ => context.Trace(MessageCode.IdentifierNotFound, $"Couldn't find any member or instance function '{instanceFunctionIdentifier}' for '{instanceBeingIndexed}' of type <{this}>")
                });

        IFunction IInstancable<IFunction>.GetDefinition(CompilationContext compilationContext) => this;
    }
}