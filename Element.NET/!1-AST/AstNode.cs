using Lexico;

namespace Element.AST
{
    public abstract class AstNode
    {
#pragma warning disable 8618
        // ReSharper disable UnusedAutoPropertyAccessor.Global
        [Location] public int IndexInSource { get; protected set; } // Filled by Lexico
        [UserObject] public SourceInfo SourceInfo { get; protected set; } // Filled by Lexico
        // ReSharper restore UnusedAutoPropertyAccessor.Global
#pragma warning restore 8618
        
        public TraceSite MakeTraceSite(string what)
        {
            var (line, column, lineCharacterIndex) = SourceInfo.CountLinesAndColumns(IndexInSource);
            return new TraceSite(what, SourceInfo.Name, line, lineCharacterIndex);
        }

        public Result Validate(CompilationContext context)
        {
            if (context.SourceContext.CompilationInput.SkipValidation || _hasBeenValidated) return Result.Success;
            var resultBuilder = new ResultBuilder(context);
            Validate(resultBuilder, context);
            return resultBuilder.ToResult();
        }

        private bool _hasBeenValidated;

        public void Validate(ResultBuilder resultBuilder, CompilationContext context)
        {
            if (context.SourceContext.CompilationInput.SkipValidation || _hasBeenValidated) return;
            ValidateImpl(resultBuilder, context);
            _hasBeenValidated = true;
        }

        protected abstract void ValidateImpl(ResultBuilder resultBuilder, CompilationContext context);
    }
}