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
                    /*new ArrayIntrinsic(),
                    new FoldIntrinsic(),
                    new MemberwiseIntrinsic(),
                    new ForIntrinsic(),
                    new PersistIntrinsic()*/
                }.Concat(Enum.GetValues(typeof(Binary.Op))
                             .Cast<Binary.Op>()
                             .Select(o => new BinaryIntrinsic(o)))
                /*.Concat(Enum.GetValues(typeof(Unary.Op))
                            .Cast<Unary.Op>()
                            .Select(o => new UnaryIntrinsic(o)))*/)
            {
                _intrinsics.Add(intrinsic.FullPath, intrinsic);
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
    public abstract class DeclaredItem : IIdentifiable
    {
        public abstract Identifier Identifier { get; }
        public abstract bool Validate(CompilationContext compilationContext);
        public abstract void Initialize(DeclaredScope parent);
    }

    public abstract class DeclaredItem<TImplementingIntrinsic> : DeclaredItem
        where TImplementingIntrinsic : Intrinsic
    {
        [Literal("intrinsic"), Optional] protected string Intrinsic;
        [IndirectLiteral(nameof(Qualifier)), WhitespaceSurrounded] protected Unnamed _;
        [Term] protected Identifier ParsedIdentifier;
        [Optional] protected PortList PortList;
        [Optional] protected Type Type;
        [IndirectAlternative(nameof(BodyAlternatives)), WhitespaceSurrounded, MultiLine] protected object Body;

        public override Identifier Identifier => ParsedIdentifier;
        public override string ToString() => $"{FullPath}{PortList}{Type}";


        public override void Initialize(DeclaredScope parent)
        {
            Parent = parent ?? throw new ArgumentNullException(nameof(parent));
            // ReSharper disable once ConstantConditionalAccessQualifier
            Child?.Initialize(parent, this);
        }

        protected DeclaredScope Parent { get; private set; }
        protected DeclaredScope Child => Body as Scope;
        protected string FullPath => $"{(Parent is GlobalScope ? string.Empty : $"{Parent.Identifier}.")}{ParsedIdentifier}";

        protected Port[] DeclaredInputs => PortList?.List.ToArray() ?? Array.Empty<Port>();
        protected bool IsIntrinsic => !string.IsNullOrEmpty(Intrinsic);
        // ReSharper disable once UnusedMember.Global
        protected abstract string Qualifier { get; }
        // ReSharper disable once UnusedMember.Global
        protected abstract System.Type[] BodyAlternatives { get; }
        protected virtual List<Identifier> ScopeIdentifierWhitelist { get; } = null;

        protected TImplementingIntrinsic? GetImplementingIntrinsic(CompilationContext compilationContext) =>
            IntrinsicCache.GetIntrinsic<TImplementingIntrinsic>(FullPath, compilationContext);

        protected bool ValidateScopeBody(CompilationContext compilationContext)
        {
            if (Body is Scope scope)
            {
                return scope.ValidateScope(compilationContext, ScopeIdentifierWhitelist);
            }

            return true;
        }

        protected bool ValidateIntrinsic(CompilationContext compilationContext)
        {
            var success = true;
            if (IsIntrinsic)
            {
                success &= GetImplementingIntrinsic(compilationContext) != null;
            }

            return success;
        }
    }
}