using System.Collections.Generic;
using System.Linq;
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
            && Verbosity >= level
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

        public void PushDeclaration(Declaration declaration) => _declarationStack.Push(declaration);
        public void PopDeclaration() => _declarationStack.Pop();
        public Declaration PeekDeclaration() => _declarationStack.Peek();
        public string CurrentDeclarationLocation => _declarationStack.Aggregate("<root>", (s, declaration) => $"{s}.{declaration.Identifier}");

        public void PushFunction(IValue functionSignature) => _callStack.Push(functionSignature);
        public void PopFunction() => _callStack.Pop();
        public bool ContainsFunction(IValue functionSignature) => _callStack.Contains(functionSignature);

        public override MessageLevel Verbosity => SourceContext.CompilationInput.Verbosity;
        public override IReadOnlyCollection<TraceSite>? TraceStack => _traceStack;
    }
}