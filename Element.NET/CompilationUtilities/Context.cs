using System.Collections.Generic;
using Element.AST;

namespace Element
{
    /// <summary>
    /// Contains contextual information about the state of compilation.
    /// </summary>
    public class Context
    {
        private class NoScope : IScope
        {
            private readonly Context _scopelessContext;

            public NoScope(Context scopelessContext)
            {
                _scopelessContext = scopelessContext;
            }
            public Result<IValue> Lookup(Identifier id, Context context) =>
                throw new InternalCompilerException($"Context '{_scopelessContext}' has no root scope, performing lookup is not possible");
        }
        
        public Context(IScope? rootScope, CompilerOptions? compilerOptions)
        {
            RootScope = rootScope ?? new NoScope(this);
            CompilerOptions = compilerOptions.GetValueOrDefault();
        }
        public Context(SourceContext sourceContext) : this(sourceContext.GlobalScope, sourceContext.CompilerOptions) { }
        
        public static Context None { get; } = new Context(null, null);

        public IScope RootScope { get; }
        public CompilerOptions CompilerOptions { get; }
        public Stack<TraceSite> TraceStack { get; } = new Stack<TraceSite>();
        public Stack<IValue> CallStack { get; } = new Stack<IValue>();
        public Stack<Declaration> DeclarationStack { get; } = new Stack<Declaration>();
        public Stack<CompilerMessage> LookupErrorStack { get; } = new Stack<CompilerMessage>();
        
        public Result<IValue> EvaluateExpression(string expression, IScope? scopeToEvaluateIn = null) =>
            Parser.Parse<TopLevelExpression>(new SourceInfo("<input expression>", expression), this, CompilerOptions.NoParseTrace)
                  .Map(tle => tle.Expression)
                  .Check(expressionObject => expressionObject.Validate(this))
                  .Bind(expressionObject => expressionObject.ResolveExpression(scopeToEvaluateIn ?? RootScope, this));

        public CompilerMessage? Trace(MessageCode messageCode, string? contextString) =>
            (CompilerMessage.TryGetMessageLevel(messageCode, out var level), level >= CompilerOptions.Verbosity) switch
            {
                (true, true) => new CompilerMessage(messageCode, contextString, TraceStack),
                (true, false) => null, // No message should be produced 
                (false, _) => new CompilerMessage(null, MessageLevel.Error, $"Couldn't get {nameof(MessageLevel)} for {messageCode}", null),
            };

        public CompilerMessage Trace(MessageLevel messageLevel, string message) => new CompilerMessage(null, messageLevel, message, TraceStack);
    }
}