using System.Collections.Generic;
using Element.AST;

namespace Element
{
    public interface ILogger
    {
        void Log(string message);
        CompilationError LogError(int? messageCode, string context);
    }
    
    /// <summary>
    /// Contains the status of the compilation process including call stack and logging messages.
    /// </summary>
    public class CompilationContext : ILogger
    {
        public CompilationContext(SourceContext sourceContext) =>
            SourceContext = sourceContext;
        
        public SourceContext SourceContext { get; }
        public CompilationInput CompilationInput => SourceContext.CompilationInput;
        
        private Stack<TraceSite> TraceStack { get; } = new Stack<TraceSite>();
        private Stack<IFunctionSignature> FunctionStack { get; } = new Stack<IFunctionSignature>();
        
        
        public void PushTrace(TraceSite traceSite) => TraceStack.Push(traceSite);
        public void PopTrace() => TraceStack.Pop();

        public void PushFunction(IFunctionSignature functionSignature) => FunctionStack.Push(functionSignature);
        public void PopFunction() => FunctionStack.Pop();
        public bool ContainsFunction(IFunctionSignature functionSignature) => FunctionStack.Contains(functionSignature);

        public TDeclaration? GetIntrinsicsDeclaration<TDeclaration>(IIntrinsic intrinsic) where TDeclaration : Declaration =>
            SourceContext.GetIntrinsicsDeclaration<TDeclaration>(intrinsic, this);

        
        public CompilationError LogError(int? messageCode, string context)
        {
            var msg = MakeMessage(messageCode, context);
            if (!msg.MessageLevel.HasValue || msg.MessageLevel.Value >= CompilationInput.Verbosity)
            {
                CompilationInput.LogCallback?.Invoke(msg);
            }

            return CompilationError.Instance;
        }

        public void Log(string message)
        {
            var msg = MakeMessage(null, message);
            if (!msg.MessageLevel.HasValue || msg.MessageLevel.Value >= CompilationInput.Verbosity)
            {
                CompilationInput.LogCallback?.Invoke(msg);
            }
        }
        
        private CompilerMessage MakeMessage(int? messageCode, string context) => !messageCode.HasValue
                                                                                     ? new CompilerMessage(null, null, context, TraceStack?.ToArray())
                                                                                     : new CompilerMessage(messageCode.Value, CompilerMessage.TryGetMessageLevel(messageCode.Value, out var level) ? level : MessageLevel.Information, context, TraceStack?.ToArray());
    }

    public static class CompilationContextExtensions
    {
        public static TDeclaration? GetIntrinsicsDeclaration<TDeclaration>(this IIntrinsic intrinsic, CompilationContext compilationContext) where TDeclaration : Declaration =>
            compilationContext.GetIntrinsicsDeclaration<TDeclaration>(intrinsic);
    }
}