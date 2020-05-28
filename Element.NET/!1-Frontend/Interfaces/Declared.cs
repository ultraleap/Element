using Lexico;

namespace Element.AST
{
    public interface IDeclared
    {
        Declaration Declarer { get; }
        int IndexInSource { get; }
    }
    
    public abstract class Declared : IDeclared
    {
#pragma warning disable 649, 8618
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [Location] public int IndexInSource { get; private set; } // Filled by Lexico
#pragma warning restore 649
        
        public Declaration Declarer { get; private set; } // Derived classes are all instantiated by Lexico and then initialized via Initialize
#pragma warning restore 8618

        public abstract override string ToString();

        public void Initialize(Declaration declaration)
        {
            Declarer = declaration;
            InitializeImpl();
        }

        public abstract bool Validate(SourceContext sourceContext);

        protected abstract void InitializeImpl();
    }

    public static class DeclaredExtensions
    {
        public static TraceSite MakeTraceSite(this IDeclared declared, string what)
        {
            var (line, column, lineCharacterIndex) = declared.Declarer.SourceInfo.CountLinesAndColumns(declared.IndexInSource);
            return new TraceSite(what, declared.Declarer.SourceInfo.Name, line, lineCharacterIndex);
        }
    }
}