using System.Collections.Generic;
using Element.AST;

namespace Element
{
    public interface ITrace
    {
        CompilerMessage? Trace(MessageCode messageCode, string? context);
    }

    public abstract class TraceBase : ITrace
    {
        public CompilerMessage? Trace(MessageCode messageCode, string? context) =>
            CompilerMessage.TryGetMessageLevel(messageCode, out var level)
            && level >= Verbosity
                ? new CompilerMessage(messageCode, context, TraceStack)
                : null;

        public abstract MessageLevel Verbosity { get; }
        public abstract IReadOnlyCollection<TraceSite>? TraceStack { get; }
    }

    public class BasicTrace : TraceBase
    {
        public BasicTrace(MessageLevel verbosity)
        {
            Verbosity = verbosity;
        }

        public override MessageLevel Verbosity { get; }
        public override IReadOnlyCollection<TraceSite>? TraceStack => null;
    }
    
    /// <summary>
    /// Contains the status of the compilation process including call stack and logging messages.
    /// </summary>
    public class CompilationContext : TraceBase
    {
        public CompilationContext(SourceContext sourceContext) =>
            SourceContext = sourceContext;
        
        public SourceContext SourceContext { get; }
        
        private readonly Stack<TraceSite> _traceStack = new Stack<TraceSite>();
        private readonly Stack<Declaration> _declarationStack = new Stack<Declaration>();
        private readonly Stack<IValue> _callStack = new Stack<IValue>();

        public void PushTrace(in TraceSite traceSite) => _traceStack.Push(traceSite);
        public void PopTrace() => _traceStack.Pop();

        public void PushDeclaration(Declaration declaration)
        {
            _declarationStack.Push(declaration);
            _traceStack.Push(declaration.MakeTraceSite($"{declaration.GetType().Name} '{declaration.Identifier}'"));
        }

        public void PopDeclaration()
        {
            _declarationStack.Pop();
            _traceStack.Pop();
        }

        public string CurrentDeclarationLocation
        {
            get
            {
                var decls = _declarationStack.ToArray();
                string result = decls.Length > 0 ? decls[0].Identifier.String : string.Empty;
                for (var i = 1; i < decls.Length; i++)
                {
                    var decl = decls[i];
                    result = $"{result}.{decl.Identifier}";
                }

                return result;
            }
        }

        public void PushFunction(IValue functionSignature) => _callStack.Push(functionSignature);
        public void PopFunction() => _callStack.Pop();
        public bool ContainsFunction(IValue functionSignature) => _callStack.Contains(functionSignature);

        public override MessageLevel Verbosity => SourceContext.CompilationInput.Verbosity;
        public override IReadOnlyCollection<TraceSite>? TraceStack => _traceStack;
    }
}