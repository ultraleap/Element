using System.Collections;
using System.Collections.Generic;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class DeclaredNamespace : Declaration, IScope
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override string Qualifier { get; } = "namespace";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope)};
        public override IType Type => NamespaceType.Instance;
        IValue? IIndexable.this[Identifier id, bool recurse, CompilationContext compilationContext] => Child[id, recurse, compilationContext];
        public IValue this[int index] => throw new System.NotImplementedException();
        public IEnumerator<IValue> GetEnumerator() => Child.GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
        int IReadOnlyCollection<IValue>.Count => Child.Count;
    }
}