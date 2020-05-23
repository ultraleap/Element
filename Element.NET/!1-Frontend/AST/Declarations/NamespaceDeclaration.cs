using System.Collections;
using System.Collections.Generic;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class NamespaceDeclaration : Declaration, IScope
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override string Qualifier { get; } = "namespace";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope)};
        public override string ToString() => $"{Location}:Namespace";

        IValue? IIndexable.this[Identifier id, bool recurse, CompilationContext compilationContext] => Child![id, recurse, compilationContext];
        public IEnumerator<IValue> GetEnumerator() => Child!.GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
        int IReadOnlyCollection<IValue>.Count => Child!.Count;
    }
}