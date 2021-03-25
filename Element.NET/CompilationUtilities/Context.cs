using System;
using System.Collections.Generic;
using Element.AST;
using Element.CLR;
using LExpression = System.Linq.Expressions.Expression;

namespace Element
{
    /// <summary>
    /// Contains contextual information about the state of compilation including compiler flags and current trace stack, call stack etc.
    /// </summary>
    public class Context
    {
        public static Context CreateFromSourceContext(SourceContext sourceContext) => new Context(sourceContext.GlobalScope, sourceContext.CompilerOptions, sourceContext.BoundaryMap, sourceContext.GeneratedStructuralTuples);
        public static Context CreateManually(IScope? rootScope, CompilerOptions? compilerOptions, BoundaryMap? boundaryMap) => new Context(rootScope, compilerOptions, boundaryMap, new List<StructuralTuple>());

        private class NoScope : IScope
        {
            private readonly Context _scopelessContext;

            public NoScope(Context scopelessContext) => _scopelessContext = scopelessContext;

            public Result<IValue> Lookup(Identifier id, Context context) =>
                throw new InternalCompilerException($"Context '{_scopelessContext}' has no root scope, performing lookup is not possible");
        }

        protected Context(IScope? rootScope, CompilerOptions? compilerOptions, BoundaryMap? boundaryMap, List<StructuralTuple> structuralTuples)
        {
            StructuralTuples = structuralTuples;
            RootScope = rootScope ?? new NoScope(this);
            CompilerOptions = compilerOptions ?? new CompilerOptions(MessageLevel.Information);
            Aspect = CompilerOptions.CompilationAspectFunc?.Invoke(this);
            BoundaryMap = boundaryMap ?? BoundaryMap.CreateDefault();
        }
        
        public static Context None { get; } = CreateManually(null, null, null);

        public IScope RootScope { get; }
        public CompilerOptions CompilerOptions { get; }
        
        public BoundaryMap BoundaryMap { get; }
        public ICompilationAspect? Aspect { get; }
        public Stack<TraceSite> TraceStack { get; } = new Stack<TraceSite>();
        public Stack<IValue> CallStack { get; } = new Stack<IValue>();
        public Stack<UniqueValueSite<Declaration>> DeclarationStack { get; } = new Stack<UniqueValueSite<Declaration>>();
        public List<StructuralTuple> StructuralTuples { get; }
        
        public Result<IValue> EvaluateExpression(string expression, IScope? scopeToEvaluateIn = null) =>
            Parse<Expression>(expression)
                .Bind(tle => EvaluateExpression(tle, scopeToEvaluateIn));

        public Result<T> Parse<T>(string source, string? sourceName = null) =>
            Parser.Parse<TopLevel<T>>(new SourceInfo(sourceName ?? $"<top level {typeof(T)}>", source), this, CompilerOptions.NoParseTrace)
                  .Map(parsed => parsed.Object);

        public Result<IValue> EvaluateExpression(Expression expression, IScope? scopeToEvaluateIn = null) =>
            expression.Validate(this)
                      .Bind(() => expression.ResolveExpression(scopeToEvaluateIn ?? RootScope, this));

        public Result<IValue> LinqToElement(LExpression parameter) => BoundaryMap.LinqToElement(parameter, this);
        public Result<LExpression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction) => BoundaryMap.ElementToLinq(value, outputType, convertFunction, this);
        public Result<Type> ElementToClr(Struct elementStruct) => BoundaryMap.ElementToClr(elementStruct, this);
        public Result<Type> ElementToClr(IIntrinsicStructImplementation elementStruct) => BoundaryMap.ElementToClr(elementStruct, this);

        public Result SerializeClrInstance(object clrInstance, ICollection<float> floats) =>
            BoundaryMap.GetConverter(clrInstance.GetType(), this)
                       .Bind(converter => converter.SerializeClrInstance(clrInstance, floats, this));

        public CompilerMessage? Trace(string messageType, int messageCode, string? contextString) =>
            (CompilerMessage.TryGetMessageLevel(messageType, messageCode, out var level), level >= CompilerOptions.Verbosity) switch
            {
                (true, true) => new CompilerMessage(messageType, messageCode, contextString, TraceStack),
                (true, false) => null, // No message should be produced
                (false, _) => new CompilerMessage(messageType, null, MessageLevel.Error, $"Couldn't get {nameof(MessageLevel)} for {messageCode}", null),
            };

        public CompilerMessage? Trace(MessageLevel messageLevel, string message, string messageType = "") => messageLevel >= CompilerOptions.Verbosity
                                                                                                                 ? new CompilerMessage(messageType, null, messageLevel, message, TraceStack)
                                                                                                                 : null;
    }
}