using System.Collections.Generic;
using Element.AST;
using ResultNET;

namespace Element
{
    public interface ICompilationAspect
    {
        void BeforeDeclaration(Declaration declaration, IScope scope);
        void Declaration(Declaration declaration, IScope scope, Result<IValue> result);
        void BeforeExpression(Expression expression, IScope scope);
        void Expression(Expression expression, IScope scope, Result<IValue> result);
        void Literal(ExpressionChain expressionChain, IScope scope, Constant constant);
        void BeforeLookup(ExpressionChain expressionChain, Identifier id, IScope scope);
        void Lookup(ExpressionChain expressionChain, Identifier id, IScope scope, Result<IValue> result);
        void BeforeIndex(ExpressionChain expressionChain, IValue valueBeingIndexed, IScope scope, ExpressionChain.IndexingExpression expr);
        void Index(ExpressionChain expressionChain, IValue valueBeingIndexed, IScope scope, ExpressionChain.IndexingExpression expr, Result<IValue> result);
        void BeforeCall(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments);
        void Call(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments, Result<IValue> result);
        void BeforeCallArgument(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope);
        void CallArgument(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope, Result<IValue> result);
        void BeforeExpressionBody(ExpressionBody expression, IScope scope);
        void ExpressionBody(ExpressionBody expression, IScope scope, Result<IValue> result);
        void BeforeScopeBody(FunctionBlock functionBlock, IScope scope);
        void ScopeBody(FunctionBlock functionBlock, IScope scope, Result<IValue> result);
        void BeforeInputPort(Port port, Expression? constraintExpression, IScope scope);
        void InputPort(Port port, Expression? constraintExpression, IScope scope, Result<IValue> result);
        void BeforeDefaultArgument(Port port, ExpressionBody? expressionBody, IScope scope);
        void DefaultArgument(Port port, ExpressionBody? expressionBody, IScope scope, Result<IValue> result);
        void BeforeReturnConstraint(PortConstraint? constraint, IScope scope);
        void ReturnConstraint(PortConstraint? constraint, IScope scope, Result<IValue> result);
    }
}