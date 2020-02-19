using System;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    public abstract class Item : IIdentifiable
    {
        public abstract Identifier Identifier { get; }
        public abstract bool Validate(CompilationContext compilationContext);

        public void Initialize(DeclaredScope parent)
        {
            Parent = parent ?? throw new ArgumentNullException(nameof(parent));
            // ReSharper disable once ConstantConditionalAccessQualifier
            Child?.Initialize(parent, this);
        }

        public DeclaredScope Parent { get; private set; }
        protected abstract DeclaredScope Child { get; }

        public string FullPath => $"{(Parent is GlobalScope ? string.Empty : $"{Parent.Identifier}.")}{Identifier}";
    }
}