using System;
using System.Collections.Generic;
using System.Linq;
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
        protected Port[] DeclaredInputs => string.IsNullOrEmpty(IntrinsicQualifier)
            ? PortList?.List.ToArray() ?? Array.Empty<Port>() // Not intrinsic so if there's no port list it's an empty array
            : PortList?.List.ToArray() ?? ImplementingIntrinsic<IFunction>(null)?.Inputs;
        protected Port DeclaredOutput { get; private set; }
        protected virtual Identifier[] ScopeIdentifierWhitelist { get; } = null;
        protected virtual Identifier[] ScopeIdentifierBlacklist { get; } = null;

        internal Declaration Clone(IScope newParent)
        {
            var clone = (Declaration)MemberwiseClone();
            clone.Initialize(newParent);
            return clone;
        }

        internal virtual bool Validate(SourceContext sourceContext)
        {
            var success = true;
            if (Body is Scope scope) success &= scope.ValidateScope(sourceContext, ScopeIdentifierBlacklist, ScopeIdentifierWhitelist);
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
            ParentScope = parent ?? throw new ArgumentNullException(nameof(parent));
            ChildScope?.Initialize(this);
            
            DeclaredType?.Initialize(ChildScope ?? ParentScope);
            DeclaredOutput = Port.ReturnPort(DeclaredType);
            foreach (var port in PortList?.List ?? Enumerable.Empty<Port>())
            {
                port.Initialize(ChildScope ?? ParentScope);
            }
        }

        public string Location => ParentScope switch
        {
            GlobalScope _ => Identifier,
            IDeclared d => $"{d.Declarer.Identifier.Value}.{Identifier.Value}",
            _ => throw new InternalCompilerException("Couldn't construct location of declaration")
        };
        public IScope ParentScope { get; private set; }
        public Scope? ChildScope => Body as Scope;

        protected TIntrinsic? ImplementingIntrinsic<TIntrinsic>(Context? context)
            where TIntrinsic : class, IValue
        {
            switch (_intrinsics.TryGetValue(Location, out var intrinsic), intrinsic)
            {
                case (true, TIntrinsic t):
                    return t;
                case (false, _):
                    context?.LogError(4, $"Intrinsic '{Location}' is not implemented");
                    return null;
                case (true, _):
                    context?.LogError(14, $"Found intrinsic '{Location}' but it is not '{typeof(TIntrinsic)}'");
                    return null;
            }
        }

        static Declaration()
        {
            foreach (var intrinsic in new IIntrinsic[]
                {
                    AnyConstraint.Instance,
                    NumType.Instance,
                    BoolType.Instance,
                    ListType.Instance,
                    new ForIntrinsic(),
                    new FoldIntrinsic(),
                    new ListIntrinsic(),
                    new InferIntrinsic(),
                    new MemberwiseIntrinsic(),
                    new PersistIntrinsic()
                }.Concat(Enum.GetValues(typeof(NullaryIntrinsics.Value))
                    .Cast<NullaryIntrinsics.Value>()
                    .Select(v => new NullaryIntrinsics(v)))
                .Concat(Enum.GetValues(typeof(Unary.Op))
                    .Cast<Unary.Op>()
                    .Select(o => new UnaryIntrinsic(o)))
                .Concat(Enum.GetValues(typeof(Binary.Op))
                    .Cast<Binary.Op>()
                    .Select(o => new BinaryIntrinsic(o))))
            {
                _intrinsics.Add(intrinsic.Location, intrinsic);
            }
        }

        private static readonly Dictionary<string, IValue> _intrinsics = new Dictionary<string, IValue>();
    }
}