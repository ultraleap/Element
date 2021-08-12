using System.Collections.Generic;
using Element.AST;
using ResultNET;

namespace Element
{
    public abstract class CompilationAspectBase : ICompilationAspect
    {
        public virtual void BeforeDeclaration(Declaration declaration, IScope scope) { }
        public virtual void Declaration(Declaration declaration, IScope scope, Result<IValue> result) { }
        public virtual void BeforeExpression(Expression expression, IScope scope) { }
        public virtual void Expression(Expression expression, IScope scope, Result<IValue> result) { }
        public virtual void Literal(ExpressionChain expressionChain, IScope scope, Constant constant) { }
        public virtual void BeforeLookup(ExpressionChain expressionChain, Identifier id, IScope scope) { }
        public virtual void Lookup(ExpressionChain expressionChain, Identifier id, IScope scope, Result<IValue> result) { }
        public virtual void BeforeIndex(ExpressionChain expressionChain, IValue valueBeingIndexed, IScope scope, ExpressionChain.IndexingExpression expr) { }
        public virtual void Index(ExpressionChain expressionChain, IValue valueBeingIndexed, IScope scope, ExpressionChain.IndexingExpression expr, Result<IValue> result) { }
        public virtual void BeforeCall(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments) { }
        public virtual void Call(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments, Result<IValue> result) { }
        public virtual void BeforeCallArgument(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope) { }
        public virtual void CallArgument(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope, Result<IValue> result) { }
        public virtual void BeforeExpressionBody(ExpressionBody expression, IScope scope) { }
        public virtual void ExpressionBody(ExpressionBody expression, IScope scope, Result<IValue> result) { }
        public virtual void BeforeScopeBody(FunctionBlock functionBlock, IScope scope) { }
        public virtual void ScopeBody(FunctionBlock functionBlock, IScope scope, Result<IValue> result) { }
        public virtual void BeforeInputPort(Port port, Expression? constraintExpression, IScope scope) { }
        public virtual void InputPort(Port port, Expression? constraintExpression, IScope scope, Result<IValue> result) { }
        public virtual void BeforeDefaultArgument(Port port, ExpressionBody? expressionBody, IScope scope) { }
        public virtual void DefaultArgument(Port port, ExpressionBody? expressionBody, IScope scope, Result<IValue> result) { }
        public virtual void BeforeReturnConstraint(PortConstraint? constraint, IScope scope) { }
        public virtual void ReturnConstraint(PortConstraint? constraint, IScope scope, Result<IValue> result) { }
    }
}