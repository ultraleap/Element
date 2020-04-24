using System.Collections.Generic;

namespace Element.AST
{
    public sealed class StructInstance : ScopeBase, IValue, IReadOnlyList<IValue>
    {
        public IType Type { get; }
        private DeclaredStruct DeclaringStruct { get; }

        public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
            IndexCache(id)
            ?? DeclaringStruct.ResolveInstanceFunction(id, this, compilationContext);

        public StructInstance(DeclaredStruct declaringStruct, Port[] inputs, IValue[] memberValues, IType? instanceType = default)
        {
            DeclaringStruct = declaringStruct;
            Type = instanceType ?? declaringStruct;
            SetRange(memberValues.WithoutDiscardedArguments(inputs));
        }

        public IValue this[int index] => IndexCache(index);
    }
}