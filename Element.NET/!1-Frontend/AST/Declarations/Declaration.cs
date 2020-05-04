using System;
using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    public abstract class Declaration : IValue
    {
#pragma warning disable 649
        // ReSharper disable UnassignedField.Global
        [IndirectLiteral(nameof(IntrinsicQualifier))] protected Unnamed _;
        [IndirectLiteral(nameof(Qualifier)), WhitespaceSurrounded] protected Unnamed __;
        [Term] public Identifier Identifier;
        [Optional] protected PortList? PortList;
        [Optional] protected TypeAnnotation? DeclaredType;
        [IndirectAlternative(nameof(BodyAlternatives)), WhitespaceSurrounded, MultiLine] protected object Body;
        // ReSharper restore UnassignedField.Global
#pragma warning restore 649

        // ReSharper disable UnusedMember.Global
        protected abstract string IntrinsicQualifier { get; }
        protected abstract string Qualifier { get; }
        protected abstract Type[] BodyAlternatives { get; }
        // ReSharper restore UnusedMember.Global

        public override string ToString() => $"{Location}{DeclaredType}";

        protected bool HasDeclaredInputs => DeclaredInputs.Length > 0;
        protected Port[] DeclaredInputs { get; private set; }
        protected Port DeclaredOutput { get; private set; }
        protected virtual Identifier[] ScopeIdentifierWhitelist { get; } = null;
        protected virtual Identifier[] ScopeIdentifierBlacklist { get; } = null;

        private sealed class StubDeclaration : Declaration
        {
            protected override string IntrinsicQualifier => string.Empty;
            protected override string Qualifier => string.Empty;
            protected override Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
            public override IType Type => FunctionType.Instance;
        }

        public static Declaration MakeStubDeclaration(Identifier id, object body, CompilationContext compilationContext) =>
            new StubDeclaration
            {
                Identifier = id,
                Body = body,
                Parent = compilationContext.SourceContext.GlobalScope,
            };

        internal Declaration Clone(IScope newParent)
        {
            var clone = (Declaration)MemberwiseClone();
            clone.Initialize(newParent);
            return clone;
        }

        internal virtual bool Validate(SourceContext sourceContext)
        {
            var success = true;
            switch (Body)
            {
                case Scope scope:
                    success &= scope.ValidateScope(sourceContext, ScopeIdentifierBlacklist, ScopeIdentifierWhitelist);
                    break;
                case ExpressionBody expressionBody:
                    success &= expressionBody.Expression.Validate(sourceContext);
                    break;
            }
            if (PortList?.List.Count > 0)
            {
                var distinctPortIdentifiers = new HashSet<string>();
                foreach (var port in PortList.List)
                {
                    if (!(port.Identifier is { } id)) continue;
                    if (!distinctPortIdentifiers.Add(id))
                    {
                        sourceContext.LogError(2, $"Cannot add duplicate identifier '{id}'");
                        success = false;
                    }

                    if (!sourceContext.ValidateIdentifier(id))
                    {
                        success = false;
                    }
                }
            }

            return success;
        }

        public bool HasBeenValidated { get; set; }
        public abstract IType Type { get; }

        internal void Initialize(IScope parent)
        {
            Parent = parent ?? throw new ArgumentNullException(nameof(parent));
            Child?.Initialize(this);
            DeclaredInputs = string.IsNullOrEmpty(IntrinsicQualifier)
                                 ? PortList?.List.ToArray() ?? Array.Empty<Port>() // Not intrinsic so if there's no port list it's an empty array
                                 : PortList?.List.ToArray() ?? ImplementingIntrinsic<IFunctionSignature>(null)?.Inputs;
            DeclaredOutput = Port.ReturnPort(DeclaredType);
            if (Body is ExpressionBody expressionBody) expressionBody.Expression.Initialize(this);
        }

        public string Location => Parent switch
        {
            GlobalScope _ => Identifier,
            IDeclared d => $"{d.Declarer.Identifier.Value}.{Identifier.Value}",
            _ => throw new InternalCompilerException("Couldn't construct location of declaration")
        };
        public Scope? Child => Body as Scope;
        public IScope Parent { get; private set; }

        protected TIntrinsic? ImplementingIntrinsic<TIntrinsic>(Context? context)
            where TIntrinsic : class, IValue
            => IntrinsicCache.GetByLocation<TIntrinsic>(Location, context);
    }
}