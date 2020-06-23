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

        public abstract Result<Port[]> Fields { get; }
        public Result<IValue> this[Identifier id, bool recurse, CompilationContext context] => (Child ?? Parent)[id, recurse, context];
        public int Count => Child?.Count ?? 0;
        public IEnumerator<IValue> GetEnumerator() => Child?.GetEnumerator() ?? Enumerable.Empty<IValue>().GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();


        Port[] IFunctionSignature.Inputs => DeclaredInputs;
        Port IFunctionSignature.Output => Port.ReturnPort(this);
        public abstract Result<ISerializableValue> DefaultValue(CompilationContext compilationContext);

        public Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context)
        {
            // TODO: Push function, check arguments, etc
        }

        protected abstract Result<IValue> Construct(IEnumerable<IValue> fields);

        public abstract Result<bool> MatchesConstraint(IValue value, CompilationContext compilationContext);
        public Result<IValue> ResolveInstanceFunction(Identifier instanceFunctionIdentifier, IValue instanceBeingIndexed, CompilationContext context) =>
            this[instanceFunctionIdentifier, false, context]
                .Bind(v => v switch
                {
                    FunctionDeclaration instanceFunction when instanceFunction.IsNullary() => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"Constant '{instanceFunction.Location}' cannot be accessed by indexing an instance"),
                    IFunctionSignature function => function.Inputs[0].ResolveConstraint(this, context).Bind(constraint => constraint switch
                    {
                        {} when constraint == this => function.ResolveCall(new[] {instanceBeingIndexed}, true, context),
                        _ => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"Found function '{function}' <{function.Inputs[0]}> must be of type <:{Identifier}> to be used as an instance function")
                    }),
                    Declaration notInstanceFunction => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"'{notInstanceFunction.Location}' is not a function"),
                    {} notInstanceFunction => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"'{notInstanceFunction}' found by indexing '{instanceBeingIndexed}' is not a function"),
                    _ => context.Trace(MessageCode.IdentifierNotFound, $"Couldn't find any member or instance function '{instanceFunctionIdentifier}' for '{instanceBeingIndexed}' of type <{this}>")
                });

        public StructInstance CreateInstance(IEnumerable<IValue> members) => new StructInstance(this, DeclaredInputs, members);
        IFunctionSignature IInstancable<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;
    }
}