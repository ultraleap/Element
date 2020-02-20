using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public abstract class CallableDeclaration : Item
    {
        static CallableDeclaration()
        {
            _intrinsics.Add("Any", AnyType.Instance);
            _intrinsics.Add("Num", NumType.Instance);


            /*
            _functions = new List<INamedFunction>
            {
                new ArrayIntrinsic(),
                new FoldIntrinsic(),
                new MemberwiseIntrinsic(),
                new ForIntrinsic(),
                new PersistIntrinsic()
            };*/
            foreach (var fun in Enum.GetValues(typeof(Binary.Op))
                .Cast<Binary.Op>()
                .Select(o => new BinaryIntrinsic(o)))
            {
                _intrinsics.Add(fun.FullPath, fun);
            }
            //_functions.AddRange(Enum.GetValues(typeof(Unary.Op)).Cast<Unary.Op>().Select(o => new UnaryIntrinsic(o)));
        }

        private static readonly Dictionary<string, IValue> _intrinsics = new Dictionary<string, IValue>();

        public IValue? ImplementingIntrinsic =>
            _intrinsics.TryGetValue(FullPath, out var implementingIntrinsic)
                ? implementingIntrinsic
                : null;
    }

    public abstract class CallableDeclaration<TBody> : CallableDeclaration
    {
        [Literal("intrinsic"), Optional] protected string Intrinsic;
        [IndirectLiteral(nameof(Qualifier)), WhitespaceSurrounded] protected Unnamed _;
        [Term] protected Declaration Declaration;
        [Term] protected TBody Body;

        public override Identifier Identifier => Declaration.Identifier;
        public override string ToString() => Declaration.ToString();

        public Port[] DeclaredInputs => Declaration.PortList?.List.ToArray() ?? Array.Empty<Port>();
        protected bool IsIntrinsic => !string.IsNullOrEmpty(Intrinsic);
        protected abstract string Qualifier { get; }
        protected virtual List<Identifier> ScopeIdentifierWhitelist { get; }
        protected override DeclaredScope Child => Body as Scope;

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
            if (IsIntrinsic && ImplementingIntrinsic == null)
            {
                compilationContext.LogError(4, $"Intrinsic '{FullPath}' not implemented");
                success = false;
            }

            return success;
        }

        public IConstraint? FindConstraint(Type type, CompilationContext compilationContext)
        {
            if (type == null) return AnyType.Instance;
            var scopeToStartWith = Body as IScope ?? Parent;
            var value = scopeToStartWith.ResolveIndexExpressions(type.Identifier, type.IndexingExpressions, compilationContext);
            switch (value)
            {
                case IConstraint constraint:
                    return constraint;
                case null:
                    return null; // If we found null it's because nothing was found, ResolveIndexExpression will have already logged an error
                default:
                    // Last error case where we found something but it was not a type.
                    compilationContext.LogError(16, $"'{value}' is not a constraint");
                    return null;
            }
        }
    }
}