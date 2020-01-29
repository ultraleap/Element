using System.IO;
using Eto.Parse;

namespace Element
{
    public class ParseMatch
    {
        public ParseMatch(in Match match, in FileInfo sourceFile)
        {
            _match = match;
            _sourceFile = sourceFile;
        }

        public bool HasCompiled { get; private set; }

        private TraceSite MakeTraceSite(string what)
        {
            var text = ((Eto.Parse.Scanners.StringScanner) _match.Scanner).Value;
            var (line, column, _) = Parser.CountLinesAndColumns(_match.Index, text);
            return new TraceSite(what, _sourceFile, line, column);
        }

        public IValue Compile(ICompilationScope scope, CompilationContext compilationContext)
        {
            HasCompiled = true;
            if (!_match.Success) return CompilationErr.Instance;

            var itemIdentifier = _match["declIdentifier"].Text;
            compilationContext.Push(MakeTraceSite(itemIdentifier));
            var value = new Literal(itemIdentifier, _match["expressionBody", true]["expression"] switch
            {
                { } m when m["literal"] is var lit && lit.Success => (float) lit.Value,
                { } m when m["identifier"] is var id && id.Success => ((Literal) scope.Compile(
                    (string) id.Value, compilationContext)).Value
            });
            compilationContext.Pop();
            return value;
        }

        private readonly Match _match;
        private readonly FileInfo _sourceFile;
    }
}