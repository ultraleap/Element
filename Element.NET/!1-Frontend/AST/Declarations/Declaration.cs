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
        protected virtual Identifier[] ScopeIdentifierWhitelist { get; } = null;
        protected virtual Identifier[] ScopeIdentifierBlacklist { get; } = null;

        private sealed class StubDeclaration : Declaration
        {
            protected override string IntrinsicQualifier => string.Empty;
            protected override string Qualifier => string.Empty;
            protected override Type[] BodyAlternatives { get; } = {typeof(Binding), typeof(Scope), typeof(Terminal)};
            public override string ToString() => "<stub declaration>";
        }

        public static Declaration MakeStubDeclaration(Identifier id, object body, string expressionString, CompilationContext compilationContext)
        {
            var result = new StubDeclaration
            {
                Identifier = id,
                Body = body
            };
            result.Initialize(new SourceInfo(id, expressionString), compilationContext.SourceContext.GlobalScope);
            return result;
        }

        internal Declaration Clone(IScope newParent)
        {
            var clone = (Declaration)MemberwiseClone();
            clone.Initialize(SourceInfo, newParent);
            return clone;
        }

        internal bool Validate(SourceContext sourceContext)
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

            success &= PortList?.Validate(sourceContext) ?? true;
            success &= DeclaredType?.Validate(sourceContext) ?? true;
            success &= AdditionalValidation(sourceContext);
            return success;
        }
        
        protected virtual bool AdditionalValidation(SourceContext sourceContext) => true;

        public bool HasBeenValidated { get; set; }

        internal void Initialize(SourceInfo info, IScope parent)
        {
            SourceInfo = info;
            Parent = parent ?? throw new ArgumentNullException(nameof(parent));
            Child?.Initialize(this);
            DeclaredType?.Initialize(this);
            DeclaredInputs = (string.IsNullOrEmpty(IntrinsicQualifier)
                                  ? PortList?.Ports.List.ToArray() ?? Array.Empty<Port>() // Not intrinsic so if there's no port list it's an empty array
                                  : PortList?.Ports.List.ToArray() ?? ImplementingIntrinsic<IFunctionSignature>(null)?.Inputs) ?? Array.Empty<Port>();
            DeclaredOutput = Port.ReturnPort(DeclaredType);
            foreach (var port in DeclaredInputs.Append(DeclaredOutput))
            {
                port.Initialize(this);
            }
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
        public Declaration Declarer => this;

        protected TIntrinsic? ImplementingIntrinsic<TIntrinsic>(ILogger? logger)
            where TIntrinsic : class, IValue
            => IntrinsicCache.GetByLocation<TIntrinsic>(Location, logger);

    }
}