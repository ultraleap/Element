using System.Collections;
using System.Collections.Generic;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class DeclaredNamespace : Declaration, IScope, IEnumerable<IValue>
    {
        protected override string IntrinsicQualifier => string.Empty;
        protected override string Qualifier { get; } = "namespace";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope)};
        public override IType Type => NamespaceType.Instance;
        public IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] => ChildScope[id, recurse, compilationContext];
        public IEnumerator<IValue> GetEnumerator() => ChildScope.GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }
}