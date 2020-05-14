namespace Element.AST
{
    public abstract class SubExpression : Declared
    {
        public IValue ResolveSubExpression(IValue previous, IScope scope, CompilationContext compilationContext)
        {
            compilationContext.PushTrace(MakeTraceSite());
            var result = SubExpressionImpl(previous, scope, compilationContext);
            compilationContext.PopTrace();
            return result;
        }
        
        protected abstract IValue SubExpressionImpl(IValue previous, IScope scope, CompilationContext context);
    }
}