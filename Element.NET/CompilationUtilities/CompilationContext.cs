using System.Collections.Generic;
using Element.AST;

namespace Element
{
    /// <summary>
    /// Contains the status of the compilation process including call stack and logging messages.
    /// </summary>
    public class CompilationContext : Context
    {
        public CompilationContext(GlobalScope globalScope, CompilationInput compilationInput) : base(globalScope, compilationInput) { }
        private Stack<TraceSite> TraceStack { get; } = new Stack<TraceSite>();
        private Stack<ICompilableFunction> FunctionStack { get; } = new Stack<ICompilableFunction>();
        public void PushTrace(TraceSite traceSite) => TraceStack.Push(traceSite);
        public void PopTrace() => TraceStack.Pop();

        public void PushFunction(ICompilableFunction function) => FunctionStack.Push(function);
        public void PopFunction() => FunctionStack.Pop();
        public bool ContainsFunction(ICompilableFunction function) => FunctionStack.Contains(function);

        protected override CompilerMessage MakeMessage(int? messageCode, string context = default) => !messageCode.HasValue
            ? new CompilerMessage(null, null, context, TraceStack?.ToArray())
            : new CompilerMessage(messageCode.Value, CompilerMessage.GetMessageLevel(messageCode.Value), context, TraceStack?.ToArray());
    }
}