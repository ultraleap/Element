namespace Element.AST
{
    public abstract class SubExpression : Declared
    {
        public IValue ResolveSubExpression(IValue previous, IScope scope, CompilationContext compilationContext)
        {
            compilationContext.PushTrace(this.MakeTraceSite(ToString()));
            var result = SubExpressionImpl(previous, scope, compilationContext);
            compilationContext.PopTrace();
            return result;
        }
        
        protected abstract IValue SubExpressionImpl(IValue previous, IScope scope, CompilationContext context);
    }
}