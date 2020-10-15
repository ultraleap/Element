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

    public abstract class WrapperValue : IValue
    {
        public override string ToString() => WrappedValue.ToString();
        public IValue WrappedValue { get; }
        protected WrapperValue(IValue result) => WrappedValue = result;
        public virtual string TypeOf => WrappedValue.TypeOf;
        public virtual string SummaryString => WrappedValue.SummaryString;
        public virtual Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) => WrappedValue.Call(arguments, context);
        public virtual IReadOnlyList<ResolvedPort> InputPorts => WrappedValue.InputPorts;
        public virtual IValue ReturnConstraint => WrappedValue.ReturnConstraint;
        public virtual Result<IValue> Index(Identifier id, Context context) => WrappedValue.Index(id, context);
        public virtual IReadOnlyList<Identifier> Members => WrappedValue.Members;
        public virtual Result<bool> MatchesConstraint(IValue value, Context context) => WrappedValue.MatchesConstraint(value, context);
        public virtual Result<IValue> DefaultValue(Context context) => WrappedValue.DefaultValue(context);
        public virtual void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context) => WrappedValue.Serialize(resultBuilder, context);
        public virtual Result<IValue> Deserialize(Func<Instruction> nextValue, Context context) => WrappedValue.Deserialize(nextValue, context);
        public virtual bool IsFunction => WrappedValue.IsFunction;
        public virtual bool IsIntrinsic => WrappedValue.IsIntrinsic;
        public virtual bool IsIntrinsicOfType<TIntrinsicImplementation>() where TIntrinsicImplementation : IIntrinsicImplementation => WrappedValue.IsIntrinsicOfType<TIntrinsicImplementation>();
        public virtual bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic) => WrappedValue.IsSpecificIntrinsic(intrinsic);
        public IValue Inner => WrappedValue.Inner;
    }

    public class DebugCall : WrapperValue
    {
        public IValue Function { get; }
        public IReadOnlyList<IValue> Arguments { get; }

        public DebugCall(IValue function, IReadOnlyList<IValue> arguments, IValue result) : base(result)
        {
            Function = function;
            Arguments = arguments;
        }
    }

    public class DebugIndex : WrapperValue
    {
        public IValue ValueBeingIndexed { get; }
        public Identifier Identifier { get; }

        public DebugIndex(IValue valueBeingIndexed, Identifier identifier, IValue result) : base(result)
        {
            ValueBeingIndexed = valueBeingIndexed;
            Identifier = identifier;
        }
    }

    public class DebugDeclaration : WrapperValue
    {
        public Declaration Declaration { get; }
        public IScope DeclaringScope { get; }

        public DebugDeclaration(Declaration declaration, IScope declaringScope, IValue result) : base(result)
        {
            Declaration = declaration;
            DeclaringScope = declaringScope;
        }
    }

    public class DebugExpression : WrapperValue
    {
        public Expression Expression { get; }
        public IScope ScopeResolvedIn { get; }

        public DebugExpression(Expression expression, IScope scopeResolvedIn, IValue result) : base(result)
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