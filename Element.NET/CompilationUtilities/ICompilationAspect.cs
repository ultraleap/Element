using System.Collections.Generic;
using Element.AST;

namespace Element
{
    public interface ICompilationAspect
    {
        Result<IValue> Declaration(Declaration declaration, IScope scope, Result<IValue> result);
        Result<IValue> Expression(Expression expression, IScope scope, Result<IValue> result);
        Result<IValue> Literal(ExpressionChain expressionChain, Constant constant);
        Result<IValue> Lookup(ExpressionChain expressionChain, Identifier id, IScope scope, Result<IValue> result);
        Result<IValue> Index(ExpressionChain expressionChain, IValue valueBeingIndexed, ExpressionChain.IndexingExpression expr, Result<IValue> result);
        Result<IValue> Call(ExpressionChain expressionChain, IValue function, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments, Result<IValue> result);
        Result<IValue> CallArgument(IValue function, Expression argumentExpression, ResolvedPort port, IScope scope, Result<IValue> result);
        Result<IValue> FunctionBody(Expression expression, IScope scope, Result<IValue> result);
        Result<IValue> InputPort(Port port, Expression? constraintExpression, IScope scope, Result<IValue> result);
        Result<IValue> DefaultArgument(Port port, ExpressionBody? expressionBody, IScope scope, Result<IValue> result);
        Result<IValue> ReturnConstraint(PortConstraint? constraint, IScope scope, Result<IValue> result);
    }
}