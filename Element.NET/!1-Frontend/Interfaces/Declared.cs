using Lexico;

namespace Element.AST
{
    public interface IDeclared
    {
        Declaration Declarer { get; }
    }
    
    public abstract class Declared : IDeclared
    {
        // ReSharper disable once UnassignedField.Global
        // Filled by Lexico
        [Location] protected int ExpressionIndex;
        
        public Declaration Declarer { get; private set; }
        public void Initialize(Declaration declaration)
        {
            Declarer = declaration;
            InitializeImpl();
        }

        public abstract bool Validate(SourceContext sourceContext);

        protected abstract void InitializeImpl();

        protected TraceSite MakeTraceSite()
        {
            var (line, column, lineCharacterIndex) = Declarer.SourceInfo.CountLinesAndColumns(ExpressionIndex);
            return new TraceSite(ToString(), Declarer.SourceInfo.Name, line, lineCharacterIndex);
        }
    }
}