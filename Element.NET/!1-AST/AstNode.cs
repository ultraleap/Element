using System.Collections.Generic;
using Lexico;

namespace Element.AST
{
    /// <summary>
    /// Base class for any Element AST node.
    /// </summary>
    public abstract class AstNode : ISourceLocation
    {
#pragma warning disable 8618
        // ReSharper disable UnusedAutoPropertyAccessor.Global
        [Location] public int IndexInSource { get; protected set; } // Filled by Lexico
        [UserObject] public SourceInfo SourceInfo { get; protected set; } // Filled by Lexico
        // ReSharper restore UnusedAutoPropertyAccessor.Global
#pragma warning restore 8618

        public Result Validate(Context context)
        {
            if (context.CompilerOptions.SkipValidation || _hasBeenValidated) return Result.Success;
            var resultBuilder = new ResultBuilder(context);
            Validate(resultBuilder, context);
            return resultBuilder.ToResult();
        }

        private bool _hasBeenValidated;

        public void Validate(ResultBuilder resultBuilder, Context context)
        {
            if (context.CompilerOptions.SkipValidation || _hasBeenValidated) return;
            ValidateImpl(resultBuilder, context);
            _hasBeenValidated = true;
        }

        protected abstract void ValidateImpl(ResultBuilder builder, Context context);
    }
    
    public readonly struct UniqueValueSite<TNode>
        where TNode : AstNode
    {
        public bool Equals(UniqueValueSite<TNode> other) => EqualityComparer<TNode>.Default.Equals(AstNode, other.AstNode) && Scope.Equals(other.Scope);

        public override bool Equals(object? obj) => obj is UniqueValueSite<TNode> other && Equals(other);

        public override int GetHashCode()
        {
            unchecked
            {
                return (EqualityComparer<TNode>.Default.GetHashCode(AstNode) * 397) ^ Scope.GetHashCode();
            }
        }

        public UniqueValueSite(TNode astNode, IScope scope)
        {
            AstNode = astNode;
            Scope = scope;
        }

        public TNode AstNode { get; }
        public IScope Scope { get; }
    }

    public interface ISourceLocation
    {
        int IndexInSource { get; }
        SourceInfo SourceInfo { get; }
    }

    public static class SourceLocatableExtensions
    {
        public static TraceSite MakeTraceSite(this ISourceLocation location, string what)
        {
            var (line, _, lineCharacterIndex) = location.SourceInfo.CalculateLineAndColumnFromIndex(location.IndexInSource);
            return new TraceSite(what, location.SourceInfo.Name, line, lineCharacterIndex);
        }
    }
}