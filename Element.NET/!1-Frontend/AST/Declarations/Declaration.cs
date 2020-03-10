using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IIntrinsic : IValue
    {
        string Location { get; }
    }

    public interface IDeclared
    {
        Declaration Declarer { get; }
    }
    
    internal static class IntrinsicCache
    {
        static IntrinsicCache()
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
                }.Concat(Enum.GetValues(typeof(Binary.Op))
                             .Cast<Binary.Op>()
                             .Select(o => new BinaryIntrinsic(o)))
                .Concat(Enum.GetValues(typeof(Unary.Op))
                            .Cast<Unary.Op>()
                            .Select(o => new UnaryIntrinsic(o)))
                .Concat(Enum.GetValues(typeof(NumIntrinsicValues.Value))
                            .Cast<NumIntrinsicValues.Value>()
                            .Select(v => new NumIntrinsicValues(v))))
            {
                _intrinsics.Add(intrinsic.Location, intrinsic);
            }
        }

        private static readonly Dictionary<string, IValue> _intrinsics = new Dictionary<string, IValue>();

        public static TIntrinsic? GetIntrinsic<TIntrinsic>(string fullPath, Context context)
            where TIntrinsic : class, IValue
        {
            switch (_intrinsics.TryGetValue(fullPath, out var intrinsic), intrinsic)
            {
                case (true, TIntrinsic t):
                    return t;
                case (false, _):
                    context.LogError(4, $"Intrinsic '{fullPath}' is not implemented");
                    return null;
                case (true, _):
                    context.LogError(14, $"Found intrinsic '{fullPath}' but it is not '{typeof(TIntrinsic)}'");
                    return null;
            }
        }
    }

    [WhitespaceSurrounded, MultiLine]
    public abstract class Declaration : IValue
    {
#pragma warning disable 649
        // ReSharper disable UnassignedField.Global
        [IndirectLiteral(nameof(IntrinsicQualifier))] protected Unnamed _;
        [IndirectLiteral(nameof(Qualifier)), WhitespaceSurrounded] protected Unnamed __;
        [Term] public Identifier Identifier;
        [Optional] protected PortList? PortList;
        [Optional] protected Type? DeclaredType;
        [IndirectAlternative(nameof(BodyAlternatives)), WhitespaceSurrounded, MultiLine] protected object Body;
        // ReSharper restore UnassignedField.Global
#pragma warning restore 649

        // ReSharper disable UnusedMember.Global
        protected abstract string IntrinsicQualifier { get; }
        protected abstract string Qualifier { get; }
        protected abstract System.Type[] BodyAlternatives { get; }
        // ReSharper restore UnusedMember.Global

        public override string ToString() => $"{Location}{DeclaredType}";

        protected bool HasDeclaredInputs => DeclaredInputs?.Length > 0;
        protected Port[]? DeclaredInputs => string.IsNullOrEmpty(IntrinsicQualifier)
            ? PortList?.List.ToArray() ?? Array.Empty<Port>() // Not intrinsic so if there's no port list it's just
            : PortList?.List.ToArray();
        protected virtual Identifier[] ScopeIdentifierWhitelist { get; } = null;
        protected virtual Identifier[] ScopeIdentifierBlacklist { get; } = null;

        public Declaration Clone(IScope newParent)
        {
            var clone = (Declaration)MemberwiseClone();
            clone.Initialize(newParent);
            return clone;
        }

        public abstract bool Validate(SourceContext sourceContext);
        public bool HasBeenValidated { get; set; }
        public abstract IType Type { get; }

        public void Initialize(IScope parent)
        {
            ParentScope = parent ?? throw new ArgumentNullException(nameof(parent));
            ChildScope?.Initialize(this);
        }

        public string Location => ParentScope switch
        {
            GlobalScope _ => Identifier,
            IDeclared d => $"{d.Declarer}.{Identifier}",
            _ => throw new InternalCompilerException("")
        };
        public IScope ParentScope { get; private set; }
        protected Scope? ChildScope => Body as Scope;

        protected TIntrinsic? ImplementingIntrinsic<TIntrinsic>(Context context)
            where TIntrinsic : class, IValue =>
            IntrinsicCache.GetIntrinsic<TIntrinsic>(Location, context);

        protected bool ValidateScopeBody(SourceContext sourceContext) =>
            !(Body is Scope scope) || scope.ValidateScope(sourceContext, ScopeIdentifierBlacklist, ScopeIdentifierWhitelist);
    }
}