using System.Collections.Generic;
using Element.AST;
using ResultNET;

namespace Element
{
    public class AggregateCompilationAspect : ICompilationAspect
    {
        public IList<ICompilationAspect> Aspects { get; } = new List<ICompilationAspect>();

        public void BeforeDeclaration(Declaration declaration, IScope scope)
        {
            foreach (var aspect in Aspects) aspect.BeforeDeclaration(declaration, scope);
        }

        public void Declaration(Declaration declaration, IScope scope, Result<IValue> result)
        {
            foreach (var aspect in Aspects) aspect.Declaration(declaration, scope, result);
        }

        public void BeforeExpression(Expression expression, IScope scope)
        {
            foreach (var aspect in Aspects) aspect.BeforeExpression(expression, scope);
        }

        public void Expression(Expression expression, IScope scope, Result<IValue> result)
        {
            foreach (var aspect in Aspects) aspect.Expression(expression, scope, result);
        }

        public void Literal(ExpressionChain expressionChain, IScope scope, Constant constant)
        {
            foreach (var aspect in Aspects) aspect.Literal(expressionChain, scope, constant);
        }

        public void BeforeLookup(ExpressionChain expressionChain, Identifier id, IScope scope)
        {
            foreach (var aspect in Aspects) aspect.BeforeLookup(expressionChain, id, scope);
        }

        public void Lookup(ExpressionChain expressionChain, Identifier id, IScope scope, Result<IValue> result)
        {
            foreach (var aspect in Aspects) aspect.Lookup(expressionChain, id, scope, result);
        }

        public void BeforeIndex(ExpressionChain expressionChain, IValue valueBeingIndexed, IScope scope, ExpressionChain.IndexingExpression expr)
        {
            foreach (var aspect in Aspects) aspect.BeforeIndex(expressionChain, valueBeingIndexed, scope, expr);
        }

        public void Index(ExpressionChain expressionChain, IValue valueBeingIndexed, IScope scope, ExpressionChain.IndexingExpression expr, Result<IValue> result)
        {
            foreach (var aspect in Aspects) aspect.Index(expressionChain, valueBeingIndexed, scope, expr, result);
        }

        public void BeforeCall(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments)
        {
            foreach (var aspect in Aspects) aspect.BeforeCall(expressionChain, function, scope, expression, arguments);
        }

        public void Call(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments, Result<IValue> result)
        {
            foreach (var aspect in Aspects) aspect.Call(expressionChain, function, scope, expression, arguments, result);
        }

        public void BeforeCallArgument(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope)
        {
            foreach (var aspect in Aspects) aspect.BeforeCallArgument(function, argumentExpression, port, scope);
        }

        public void CallArgument(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope, Result<IValue> result)
        {
            foreach (var aspect in Aspects) aspect.CallArgument(function, argumentExpression, port, scope, result);
        }

        public void BeforeExpressionBody(ExpressionBody expression, IScope scope)
        {
            foreach (var aspect in Aspects) aspect.BeforeExpressionBody(expression, scope);
        }

        public void ExpressionBody(ExpressionBody expression, IScope scope, Result<IValue> result)
        {
            foreach (var aspect in Aspects) aspect.ExpressionBody(expression, scope, result);
        }

        public void BeforeScopeBody(FunctionBlock functionBlock, IScope scope)
        {
            foreach (var aspect in Aspects) aspect.BeforeScopeBody(functionBlock, scope);
        }

        public void ScopeBody(FunctionBlock functionBlock, IScope scope, Result<IValue> result)
        {
            foreach (var aspect in Aspects) aspect.ScopeBody(functionBlock, scope, result);
        }

        public void BeforeInputPort(Port port, Expression? constraintExpression, IScope scope)
        {
            foreach (var aspect in Aspects) aspect.BeforeInputPort(port, constraintExpression, scope);
        }

        public void InputPort(Port port, Expression? constraintExpression, IScope scope, Result<IValue> result)
        {
            foreach (var aspect in Aspects) aspect.InputPort(port, constraintExpression, scope, result);
        }

        public void BeforeDefaultArgument(Port port, ExpressionBody? expressionBody, IScope scope)
        {
            foreach (var aspect in Aspects) aspect.BeforeDefaultArgument(port, expressionBody, scope);
        }

        public void DefaultArgument(Port port, ExpressionBody? expressionBody, IScope scope, Result<IValue> result)
        {
            foreach (var aspect in Aspects) aspect.DefaultArgument(port, expressionBody, scope, result);
        }

        public void BeforeReturnConstraint(PortConstraint? constraint, IScope scope)
        {
            foreach (var aspect in Aspects) aspect.BeforeReturnConstraint(constraint, scope);
        }

        public void ReturnConstraint(PortConstraint? constraint, IScope scope, Result<IValue> result)
        {
            foreach (var aspect in Aspects) aspect.ReturnConstraint(constraint, scope, result);
        }
    }
}