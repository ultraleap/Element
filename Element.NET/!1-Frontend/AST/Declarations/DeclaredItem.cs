using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    internal static class IntrinsicCache
    {
        static IntrinsicCache()
        {
            foreach (var intrinsic in new Intrinsic[]
                {
                    AnyConstraint.Instance,
                    NumType.Instance,
                    new ForIntrinsic(),
                    /*new ArrayIntrinsic(),
                    new FoldIntrinsic(),
                    new MemberwiseIntrinsic(),
                    new PersistIntrinsic()*/
                }.Concat(Enum.GetValues(typeof(Binary.Op))
                             .Cast<Binary.Op>()
                             .Select(o => new BinaryIntrinsic(o)))
                /*.Concat(Enum.GetValues(typeof(Unary.Op))
                            .Cast<Unary.Op>()
                            .Select(o => new UnaryIntrinsic(o)))*/)
            {
                _intrinsics.Add(intrinsic.Location, intrinsic);
            }
        }

        private static readonly Dictionary<string, Intrinsic> _intrinsics = new Dictionary<string, Intrinsic>();

        public static TIntrinsic? GetIntrinsic<TIntrinsic>(string fullPath, CompilationContext? compilationContext)
            where TIntrinsic : Intrinsic
        {
            switch (_intrinsics.TryGetValue(fullPath, out var intrinsic), intrinsic)
            {
                case (true, TIntrinsic t):
                    return t;
                case (false, _):
                    compilationContext?.LogError(4, $"Intrinsic '{fullPath}' is not implemented");
                    return null;
                case (true, _):
                    compilationContext?.LogError(14, $"Found intrinsic '{fullPath}' but it is not '{typeof(TIntrinsic)}'");
                    return null;
            }
        }
    }

    [WhitespaceSurrounded, MultiLine]
    public abstract class DeclaredItem : IScopeItem
    {
        [Literal("intrinsic"), Optional] protected string Intrinsic;
        [IndirectLiteral(nameof(Qualifier)), WhitespaceSurrounded] protected Unnamed _;
        [Term] protected Identifier ParsedIdentifier;
        [Optional] protected PortList PortList;
        [Optional] protected Type DeclaredType;
        [IndirectAlternative(nameof(BodyAlternatives)), WhitespaceSurrounded, MultiLine] protected object Body;

        // ReSharper disable once UnusedMember.Global
        protected abstract string Qualifier { get; }
        // ReSharper disable once UnusedMember.Global
        protected abstract System.Type[] BodyAlternatives { get; }
        public Identifier Identifier => ParsedIdentifier;
        public override string ToString() => $"{Location}{PortList}{DeclaredType}";

        protected Port[] DeclaredInputs => PortList?.List.ToArray() ?? Array.Empty<Port>();
        protected bool IsIntrinsic => !string.IsNullOrEmpty(Intrinsic);
        protected virtual List<Identifier> ScopeIdentifierWhitelist { get; } = null;


        protected bool ValidateScopeBody(CompilationContext compilationContext)
        {
            if (Body is Scope scope)
            {
                return scope.ValidateScope(compilationContext, ScopeIdentifierWhitelist);
            }

            return true;
        }

        public abstract bool Validate(CompilationContext compilationContext);

        public void Initialize(DeclaredScope parent)
        {
            Parent = parent ?? throw new ArgumentNullException(nameof(parent));
            // ReSharper disable once ConstantConditionalAccessQualifier
            Child?.Initialize(parent, this);
        }
        protected DeclaredScope Parent { get; private set; }
        protected DeclaredScope? Child => Body as Scope;
        public string Location => $"{(Parent is GlobalScope ? string.Empty : $"{Parent.Declarer}.")}{Identifier}";
    }

    public abstract class DeclaredItem<TImplementingIntrinsic> : DeclaredItem
        where TImplementingIntrinsic : Intrinsic
    {
        protected TImplementingIntrinsic? GetImplementingIntrinsic(CompilationContext compilationContext) =>
            IntrinsicCache.GetIntrinsic<TImplementingIntrinsic>(Location, compilationContext);

        protected bool ValidateIntrinsic(CompilationContext compilationContext)
        {
            var success = true;
            if (IsIntrinsic)
            {
                var intrinsic = GetImplementingIntrinsic(compilationContext);
                if (intrinsic != null)
                {
                    intrinsic.Declarer = this;
                }
                else
                {
                    success = false;
                }

            }

            return success;
        }
    }
}