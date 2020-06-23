using System.Collections.Generic;
using Element.AST;

namespace Element
{
    public interface ITrace
    {
        CompilerMessage? Trace(MessageCode messageCode, string context);
    }
    
    public interface ITraceSink
    {
        void Flush(CompilerMessage message);
    }
    
    /// <summary>
    /// Contains the status of the compilation process including call stack and logging messages.
    /// </summary>
    public class CompilationContext : ITrace
    {
        public CompilationContext(SourceContext sourceContext) =>
            SourceContext = sourceContext;
        
        public SourceContext SourceContext { get; }
        
        private Stack<TraceSite> TraceStack { get; } = new Stack<TraceSite>();
        private Stack<IFunction> FunctionStack { get; } = new Stack<IFunction>();

        public void PushTrace(TraceSite traceSite) => TraceStack.Push(traceSite);
        public void PopTrace() => TraceStack.Pop();

        public void PushFunction(IFunction function) => FunctionStack.Push(function);
        public void PopFunction() => FunctionStack.Pop();
        public bool ContainsFunction(IFunction function) => FunctionStack.Contains(function);

        public CompilerMessage? Trace(MessageCode messageCode, string context) =>
            CompilerMessage.TryGetMessageLevel((int)messageCode, out var level)
            && SourceContext.CompilationInput.Verbosity >= level
                ? new CompilerMessage(messageCode, context, TraceStack)
                : null;
    }
}