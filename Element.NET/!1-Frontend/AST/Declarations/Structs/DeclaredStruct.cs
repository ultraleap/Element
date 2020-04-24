using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public abstract class DeclaredStruct : Declaration, IFunction, IScope, IType
    {
        protected override string Qualifier { get; } = "struct";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope), typeof(Terminal)};
        protected override Identifier[] ScopeIdentifierBlacklist => new[]{Identifier};

        public IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] => (Child ?? Parent)[id, recurse, compilationContext];
        public int Count => Child?.Count ?? 0;
        public IEnumerator<IValue> GetEnumerator() => Child?.GetEnumerator() ?? Enumerable.Empty<IValue>().GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
        string IType.Name => Identifier;
        public override IType Type => TypeType.Instance;
        Port[] IFunctionSignature.Inputs => DeclaredInputs;
        Port IFunctionSignature.Output => Port.ReturnPort(this);
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);

        public abstract bool MatchesConstraint(IValue value, CompilationContext compilationContext);
        public IValue ResolveInstanceFunction(Identifier instanceFunctionIdentifier, IValue instanceBeingIndexed, CompilationContext compilationContext) =>
            this[instanceFunctionIdentifier, false, compilationContext] switch
            {
                DeclaredFunction instanceFunction when instanceFunction.IsNullary() => (IValue)compilationContext.LogError(22, $"Constant '{instanceFunction.Location}' cannot be accessed by indexing an instance"),
                IFunctionSignature function when function.Inputs[0].ResolveConstraint(this, compilationContext) == this => function.ResolveCall(new[]{instanceBeingIndexed}, true, compilationContext),
                IFunctionSignature function => compilationContext.LogError(22, $"Found function '{function}' <{function.Inputs[0]}> must be of type <:{Identifier}> to be used as an instance function"),
                Declaration notInstanceFunction => compilationContext.LogError(22, $"'{notInstanceFunction.Location}' is not a function"),
                {} notInstanceFunction => compilationContext.LogError(22, $"'{notInstanceFunction}' found by indexing '{instanceBeingIndexed}' is not a function"),
                _ => compilationContext.LogError(7, $"Couldn't find any member or instance function '{instanceFunctionIdentifier}' for '{instanceBeingIndexed}' of type <{this}>")
            };

        public IValue CreateInstance(IValue[] members, IType? instanceType = default) => new StructInstance(this, DeclaredInputs, members, instanceType);
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;
    }
}