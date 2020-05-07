using System.Collections.Generic;
using Element.AST;

namespace Element
{
    /// <summary>
    /// Contains the status of the compilation process including call stack and logging messages.
    /// </summary>
    public class CompilationContext : Context
    {
        public CompilationContext(SourceContext sourceContext) :base(sourceContext.CompilationInput) =>
            SourceContext = sourceContext;
        
        public SourceContext SourceContext { get; }
        private Stack<TraceSite> TraceStack { get; } = new Stack<TraceSite>();
        private Stack<IFunctionSignature> FunctionStack { get; } = new Stack<IFunctionSignature>();
        public void PushTrace(TraceSite traceSite) => TraceStack.Push(traceSite);
        public void PopTrace() => TraceStack.Pop();

        public void PushFunction(IFunctionSignature functionSignature) => FunctionStack.Push(functionSignature);
        public void PopFunction() => FunctionStack.Pop();
        public bool ContainsFunction(IFunctionSignature functionSignature) => FunctionStack.Contains(functionSignature);

        public TDeclaration? GetIntrinsicsDeclaration<TDeclaration>(IIntrinsic intrinsic) where TDeclaration : Declaration =>
            SourceContext.GetIntrinsicsDeclaration<TDeclaration>(intrinsic, this);

        protected override CompilerMessage MakeMessage(int? messageCode, string context = default) => !messageCode.HasValue
                                                                                                          ? new CompilerMessage(null, null, context, TraceStack?.ToArray())
                                                                                                          : new CompilerMessage(messageCode.Value, CompilerMessage.TryGetMessageLevel(messageCode.Value, out var level) ? level : MessageLevel.Information, context, TraceStack?.ToArray());
    }

    public static class CompilationContextExtensions
    {
        public static TDeclaration? GetIntrinsicsDeclaration<TDeclaration>(this IIntrinsic intrinsic, CompilationContext compilationContext) where TDeclaration : Declaration =>
            compilationContext.GetIntrinsicsDeclaration<TDeclaration>(intrinsic);
    }
}