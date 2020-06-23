using System;
using System.Linq;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    public abstract class Declaration : IValue, IDeclared
    {
#pragma warning disable 649
        // ReSharper disable UnassignedField.Global
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [Location] public int IndexInSource { get; private set; }
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

        public abstract override string ToString();

        public SourceInfo SourceInfo { get; private set; }

        protected bool HasDeclaredInputs => DeclaredInputs.Length > 0;
        protected Port[] DeclaredInputs { get; private set; }
        protected Port DeclaredOutput { get; private set; }
        protected virtual Identifier[] ScopeIdentifierWhitelist { get; } = Array.Empty<Identifier>();
        protected virtual Identifier[] ScopeIdentifierBlacklist { get; } = Array.Empty<Identifier>();

        private sealed class StubDeclaration : Declaration
        {
            protected override string IntrinsicQualifier => string.Empty;
            protected override string Qualifier => string.Empty;
            protected override Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
            public override string ToString() => "<stub declaration>";
        }

        public static Declaration MakeStubDeclaration(Identifier id, object body, string expressionString, IScope scope)
        {
            var result = new StubDeclaration
            {
                Identifier = id,
                Body = body
            };
            result.Initialize(new SourceInfo(id, expressionString), scope, null);
            return result;
        }

        internal Declaration Clone(IScope newParent)
        {
            var clone = (Declaration)MemberwiseClone();
            clone.Initialize(SourceInfo, newParent, null);
            return clone;
        }

        internal void Validate(ResultBuilder resultBuilder)
        {
            switch (Body)
            {
                case ExpressionBody expressionBody:
                    expressionBody.Expression.Validate(resultBuilder);
                    break;
                case Scope scope:
                    scope.Validate(resultBuilder, ScopeIdentifierBlacklist, ScopeIdentifierWhitelist);
                    break;
            }

            PortList?.Validate(resultBuilder);
            DeclaredType?.Validate(resultBuilder);
            AdditionalValidation(resultBuilder);
        }
        
        protected virtual void AdditionalValidation(ResultBuilder builder) {}

        internal void Initialize(SourceInfo info, IScope parent, IIntrinsicCache? cache)
        {
            SourceInfo = info;
            Parent = parent ?? throw new ArgumentNullException(nameof(parent));
            Child?.Initialize(this, cache);
            DeclaredType?.Initialize(this, cache);
            DeclaredInputs = PortList?.Ports.List.ToArray() ?? Array.Empty<Port>();
            DeclaredOutput = Port.ReturnPort(DeclaredType);
            foreach (var port in DeclaredInputs.Append(DeclaredOutput))
            {
                port.Initialize(this, cache);
            }
            if (Body is ExpressionBody expressionBody) expressionBody.Expression.Initialize(this, cache);
            if (!string.IsNullOrEmpty(IntrinsicQualifier)) cache?.CacheIntrinsicDeclaration(this);
        }

        public string Location => Parent switch
        {
            GlobalScope _ => Identifier,
            IDeclared d => $"{d.Declarer.Identifier.Value}.{Identifier.Value}",
            _ => throw new InternalCompilerException("Couldn't construct location of declaration")
        };
        public Scope? Child => Body as Scope;
        public IScope Parent { get; private set; }
        public Declaration Declarer => this;

    }
}