using System;
using System.Collections.Generic;
using Element.AST;
using Element.CLR;
using ResultNET;
using LExpression = System.Linq.Expressions.Expression;

namespace Element
{
    /// <summary>
    /// Contains contextual information about the state of compilation including compiler flags and current trace stack, call stack etc.
    /// </summary>
    public class Context : ITraceContext
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
            BoundaryMap = boundaryMap ?? BoundaryMap.CreateDefault();
        }

        public static Context None { get; } = CreateManually(null, null, null);

        public IScope RootScope { get; }
        public CompilerOptions CompilerOptions { get; }
        
        public BoundaryMap BoundaryMap { get; }

        public bool AddAspect(ICompilationAspect aspect)
        {
            if (Aspect.Aspects.Contains(aspect)) return false;
            Aspect.Aspects.Add(aspect);
            return true;
        }

        public bool RemoveAspect(ICompilationAspect aspect) => Aspect.Aspects.Remove(aspect);
        
        public AggregateCompilationAspect Aspect { get; } = new AggregateCompilationAspect();
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

        // TODO: Replace usages of this function with explicit message info
        public ResultMessage? Trace(MessageLevel messageLevel, string message, string messageTypePrefix = "") =>
            Trace(new MessageInfo(messageTypePrefix, null, messageLevel, null), message);

        public ResultMessage? Trace(MessageInfo info, string? message) =>
            info.Level >= CompilerOptions.Verbosity
                ? new ResultMessage(info, message, TraceStack)
                : null; // No message should be produced
    }
}