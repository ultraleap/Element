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
                    /*new MemberwiseIntrinsic(),
                    new PersistIntrinsic()*/
                }.Concat(Enum.GetValues(typeof(Binary.Op))
                             .Cast<Binary.Op>()
                             .Select(o => new BinaryIntrinsic(o)))
                /*.Concat(Enum.GetValues(typeof(Unary.Op))
                            .Cast<Unary.Op>()
                            .Select(o => new UnaryIntrinsic(o)))*/)
            {
                Intrinsics.Add(intrinsic.Location, intrinsic);
            }
        }

        private static readonly Dictionary<string, IValue> Intrinsics = new Dictionary<string, IValue>();

        public static TIntrinsic? GetIntrinsic<TIntrinsic>(string fullPath, CompilationContext? compilationContext)
            where TIntrinsic : class, IValue
        {
            switch (Intrinsics.TryGetValue(fullPath, out var intrinsic), intrinsic)
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
    public abstract class DeclaredItem : IValue
    {
#pragma warning disable 649
        // ReSharper disable UnassignedField.Global
        [IndirectLiteral(nameof(IntrinsicQualifier))] protected Unnamed _;
        [IndirectLiteral(nameof(Qualifier)), WhitespaceSurrounded] protected Unnamed __;
        [Term] protected Identifier ParsedIdentifier;
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
        
        public Identifier Identifier => ParsedIdentifier;
        public override string ToString() => $"{Location}{DeclaredType}";

        protected Port[]? DeclaredInputs => PortList?.List.ToArray() ?? Array.Empty<Port>();
        protected virtual List<Identifier> ScopeIdentifierWhitelist { get; } = null;
     
        public abstract bool Validate(CompilationContext compilationContext);
        public abstract IType Type { get; }

        public void Initialize(DeclaredScope parent)
        {
            ParentScope = parent ?? throw new ArgumentNullException(nameof(parent));
            ChildScope?.Initialize(this);
        }
        public string Location => $"{(ParentScope is GlobalScope ? string.Empty : $"{ParentScope.Declarer}.")}{Identifier}";
        protected DeclaredScope ParentScope { get; private set; }
        protected IScope? CaptureScope { get; set; }
        protected DeclaredScope? ChildScope => Body as Scope;

        public IValue? IndexRecursively(Identifier id, CompilationContext compilationContext) =>
            CaptureScope?[id, compilationContext] ?? ParentScope[id, compilationContext];

        protected TIntrinsic? ImplementingIntrinsic<TIntrinsic>(CompilationContext compilationContext)
            where TIntrinsic : class, IValue =>
            IntrinsicCache.GetIntrinsic<TIntrinsic>(Location, compilationContext);

        protected bool ValidateScopeBody(CompilationContext compilationContext) =>
            !(Body is Scope scope) || scope.ValidateScope(compilationContext, ScopeIdentifierWhitelist);
    }
}