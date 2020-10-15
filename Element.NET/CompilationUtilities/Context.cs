using System;
using System.Collections.Generic;
using Element.AST;

namespace Element
{
    public interface ICompilationAspect
    {
        Result<IValue> Index(IValue valueBeingIndexed, Identifier identifier, IValue result, Context context);
        Result<IValue> Call(IValue function, IReadOnlyList<IValue> arguments, IValue result, Context context);
        Result<IValue> Declaration(Declaration declaration, IScope scope, IValue value, Context context);
        Result<IValue> Expression(Expression expression, IScope scope, IValue value, Context context);
    }

    public abstract class DebugValue : IValue
    {
        public override string ToString() => ResolvedValue.ToString();
        public IValue ResolvedValue { get; }
        protected DebugValue(IValue value) => ResolvedValue = value;
        public string TypeOf => ResolvedValue.TypeOf;
        public string SummaryString => ResolvedValue.SummaryString;
        public Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) => ResolvedValue.Call(arguments, context);
        public IReadOnlyList<ResolvedPort> InputPorts => ResolvedValue.InputPorts;
        public IValue ReturnConstraint => ResolvedValue.ReturnConstraint;
        public Result<IValue> Index(Identifier id, Context context) => ResolvedValue.Index(id, context);
        public IReadOnlyList<Identifier> Members => ResolvedValue.Members;
        public Result<bool> MatchesConstraint(IValue value, Context context) => ResolvedValue.MatchesConstraint(value, context);
        public Result<IValue> DefaultValue(Context context) => ResolvedValue.DefaultValue(context);
        public void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context) => ResolvedValue.Serialize(resultBuilder, context);
        public Result<IValue> Deserialize(Func<Instruction> nextValue, Context context) => ResolvedValue.Deserialize(nextValue, context);
        public bool IsFunction => ResolvedValue.IsFunction;
        public bool IsIntrinsic => ResolvedValue.IsIntrinsic;
        public bool IsIntrinsicOfType<TIntrinsicImplementation>() where TIntrinsicImplementation : IIntrinsicImplementation => ResolvedValue.IsIntrinsicOfType<TIntrinsicImplementation>();
        public bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic) => ResolvedValue.IsSpecificIntrinsic(intrinsic);
        public IValue Inner => ResolvedValue.Inner;
    }

    public class DebugCall : DebugValue
    {
        public IValue Function { get; }
        public IReadOnlyList<IValue> Arguments { get; }

        public DebugCall(IValue function, IReadOnlyList<IValue> arguments, IValue result) : base(result)
        {
            Function = function;
            Arguments = arguments;
        }
    }

    public class DebugIndex : DebugValue
    {
        public IValue ValueBeingIndexed { get; }
        public Identifier Identifier { get; }

        public DebugIndex(IValue valueBeingIndexed, Identifier identifier, IValue result) : base(result)
        {
            ValueBeingIndexed = valueBeingIndexed;
            Identifier = identifier;
        }
    }

    public class DebugDeclaration : DebugValue
    {
        public Declaration Declaration { get; }
        public IScope DeclaringScope { get; }

        public DebugDeclaration(Declaration declaration, IScope declaringScope, IValue value) : base(value)
        {
            Declaration = declaration;
            DeclaringScope = declaringScope;
        }
    }

    public class DebugExpression : DebugValue
    {
        public Expression Expression { get; }
        public IScope ScopeResolvedIn { get; }

        public DebugExpression(Expression expression, IScope scopeResolvedIn, IValue value) : base(value)
        {
            Expression = expression;
            ScopeResolvedIn = scopeResolvedIn;
        }
    }

    public class DebugAspect : ICompilationAspect
    {
        //private static string GetDeclarationLocation(Context context) => context.DeclarationStack.Reverse().Aggregate(new StringBuilder(), (builder, decl) => builder.Append($".{decl.Identifier}")).ToString();
        
        public Result<IValue> Index(IValue valueBeingIndexed, Identifier identifier, IValue result, Context context) => new DebugIndex(valueBeingIndexed, identifier, result);
        public Result<IValue> Call(IValue function, IReadOnlyList<IValue> arguments, IValue result, Context context) => new DebugCall(function, arguments, result);
        public Result<IValue> Declaration(Declaration declaration, IScope scope, IValue value, Context context) => new DebugDeclaration(declaration, scope, value);
        public Result<IValue> Expression(Expression expression, IScope scope, IValue value, Context context) => new DebugExpression(expression, scope, value);
    }
    
    /// <summary>
    /// Contains contextual information about the state of compilation including compiler options and current trace stack, call stack etc.
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
        
        public Context(IScope? rootScope, CompilerOptions? compilerOptions, ICompilationAspect? aspect = null)
        {
            Aspect = aspect;
            RootScope = rootScope ?? new NoScope(this);
            CompilerOptions = compilerOptions ?? new CompilerOptions(MessageLevel.Information);
            if (Aspect == null && !CompilerOptions.ReleaseMode)
            {
                Aspect = new DebugAspect();
            }
        }
        public Context(SourceContext sourceContext) : this(sourceContext.GlobalScope, sourceContext.CompilerOptions, sourceContext.Aspect) { }
        
        public static Context None { get; } = new Context(null, null);

        public IScope RootScope { get; }
        public CompilerOptions CompilerOptions { get; }
        public ICompilationAspect? Aspect { get; }
        public Stack<TraceSite> TraceStack { get; } = new Stack<TraceSite>();
        public Stack<IValue> CallStack { get; } = new Stack<IValue>();
        public Stack<Declaration> DeclarationStack { get; } = new Stack<Declaration>();
        
        public Result<IValue> EvaluateExpression(string expression, IScope? scopeToEvaluateIn = null) =>
            Parse<Expression>(expression)
                .Bind(tle => EvaluateExpression(tle, scopeToEvaluateIn));

        public Result<T> Parse<T>(string source, string sourceName = "<input source>") =>
            Parser.Parse<TopLevel<T>>(new SourceInfo(sourceName, source), this, CompilerOptions.NoParseTrace)
                  .Map(parsed => parsed.Object);

        public Result<IValue> EvaluateExpression(Expression expression, IScope? scopeToEvaluateIn = null) =>
            expression.Validate(this)
                      .Bind(() => expression.ResolveExpression(scopeToEvaluateIn ?? RootScope, this));
        
        public CompilerMessage? Trace(string messageType, int messageCode, string? contextString) =>
            (CompilerMessage.TryGetMessageLevel(messageType, messageCode, out var level), level >= CompilerOptions.Verbosity) switch
            {
                (true, true) => new CompilerMessage(messageType, messageCode, contextString, TraceStack),
                (true, false) => null, // No message should be produced 
                (false, _) => new CompilerMessage(messageType, null, MessageLevel.Error, $"Couldn't get {nameof(MessageLevel)} for {messageCode}", null),
            };

        public CompilerMessage Trace(MessageLevel messageLevel, string message, string messageType = "") => new CompilerMessage(messageType, null, messageLevel, message, TraceStack);
    }
}