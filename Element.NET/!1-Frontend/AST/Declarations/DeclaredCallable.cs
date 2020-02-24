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

    public abstract class DeclaredCallable<TImplementingIntrinsic> : DeclaredItem
        where TImplementingIntrinsic : Intrinsic
    {
        [Literal("intrinsic"), Optional] protected string Intrinsic;
        [IndirectLiteral(nameof(Qualifier)), WhitespaceSurrounded] protected Unnamed _;
        [Term] protected Declaration Declaration;
        [IndirectAlternative(nameof(BodyAlternatives))] protected object Body;

        public override Identifier Identifier => Declaration.Identifier;
        public override string ToString() => Declaration.ToString();

        protected Port[] DeclaredInputs => Declaration.PortList?.List.ToArray() ?? Array.Empty<Port>();
        protected bool IsIntrinsic => !string.IsNullOrEmpty(Intrinsic);
        // ReSharper disable once UnusedMember.Global
        protected abstract string Qualifier { get; }
        // ReSharper disable once UnusedMember.Global
        protected abstract System.Type[] BodyAlternatives { get; }
        protected virtual List<Identifier> ScopeIdentifierWhitelist { get; } = null;
        protected override DeclaredScope Child => Body as Scope;

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