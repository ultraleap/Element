using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IScope
    {
        public abstract Result<IValue> Index(Identifier id, CompilationContext context);
        public abstract Result<IValue> Lookup(Identifier id, CompilationContext context);
        IReadOnlyList<Identifier> Members { get; }
    }
    
    public interface IDeclarationScope : IEnumerable<Declaration>, IScope {}

    public static class ScopeExtensions
    {
        public static Result<IScope> CombineScopes(this IScope scope, IScope other, IScope? parent, CompilationContext context)
        {
            var builder = new ResultBuilder<IScope>(context, default);
            foreach (var duplicate in scope.Members.Intersect(other.Members))
            {
                builder.Append(MessageCode.MultipleDefinitions, $"Cannot combine scopes '{scope}' and '{other}' due to duplicate identifier '{duplicate}'");
            }

            Result<IValue> IndexFunc(IScope _, Identifier identifier, CompilationContext context) =>
                scope.Index(identifier, context)
                     .Else(() => other.Index(identifier, context));

            builder.Result = new Scope(scope.Members.Concat(other.Members).ToArray(), IndexFunc , parent);
            return builder.ToResult();
        }
    }
}