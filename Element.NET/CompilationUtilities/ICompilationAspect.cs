using System;
using System.Collections.Generic;
using System.Linq;
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

    public class SummaryLinesAspect : CompilationAspectBase
    {
        private readonly Action<string> _lineCallback;
        private readonly Context _context;
        private int _indent;

        public SummaryLinesAspect(Action<string> lineCallback, Context context)
        {
            _lineCallback = lineCallback;
            _context = context;
        }

        private void CallbackLine(string line) => _lineCallback(line.PadLeft(_indent * 4 + line.Length));

        private void Push(string line, ISourceLocation? location)
        {
            if (location != null)
            {
                var (lineIdx, columnIdx, indexOnLine) = location.SourceInfo.CalculateLineAndColumnFromIndex(location.IndexInSource);
                line = $"{line} in {location.SourceInfo.FullName}:{lineIdx},{columnIdx}";
            }
            CallbackLine(line);
            _indent++;
        }

        private string ResultToString(Result<IValue> result) =>
            result.Bind(result => result.SerializeToFloats(_context)
                                        .Branch(floats => (Value: result, Serialized: floats),
                                                () => (Value: result, Serialized: Array.Empty<float>())))
                  .Match((tuple, messages) =>
                         {
                             var (value, serialized) = tuple;
                             var serializedString = serialized.Length > 0
                                                        ? $" <{string.Join(", ", serialized)}>"
                                                        : string.Empty;
                             return $"{value.SummaryString}{serializedString}";
                         },
                         messages => $"<errors: {string.Join(", ", messages.Where(msg => msg.Info.Level == MessageLevel.Error))}>");

        private void PopResult(Result<IValue> result)
        {
            _indent--;
            CallbackLine(ResultToString(result));
        }

        /*
        public override void BeforeDeclaration(Declaration declaration, IScope scope) => Push($"{declaration}", declaration);
        public override Result<IValue> Declaration(Declaration declaration, IScope scope, Result<IValue> result)
        {
            PopResult(result);
            return result;
        }*/
        
        /*
        public override void BeforeExpression(Expression expression, IScope scope) => Push($"{expression} <{expression.GetType()}>", expression);
        public override Result<IValue> Expression(Expression expression, IScope scope, Result<IValue> result)
        {
            PopResult(result);
            return result;
        }*/

        public override void Literal(ExpressionChain expressionChain, IScope scope, Constant constant) => CallbackLine(ResultToString(constant));

        /*
        public override void BeforeLookup(ExpressionChain expressionChain, Identifier id, IScope scope) => Push($"{id} <lookup>", expressionChain);
        public override Result<IValue> Lookup(ExpressionChain expressionChain, Identifier id, IScope scope, Result<IValue> result)
        {
            PopResult(result);
            return result;
        }*/
        
        /*
        public override void BeforeIndex(ExpressionChain expressionChain, IValue valueBeingIndexed, ExpressionChain.IndexingExpression expr) => Push($"{expr} in {expressionChain}", expr);
        public override Result<IValue> Index(ExpressionChain expressionChain, IValue valueBeingIndexed, ExpressionChain.IndexingExpression expr, Result<IValue> result)
        {
            PopResult(result);
            return result;
        }*/
        
        public override void BeforeCall(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments) => Push($"{expression} in {expressionChain}", expression);
        public override void Call(ExpressionChain expressionChain, IValue function, IScope scope, ExpressionChain.CallExpression expression, IReadOnlyList<IValue> arguments, Result<IValue> result) => PopResult(result);

        public override void BeforeCallArgument(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope) => Push($"{port?.Identifier} <call argument>", argumentExpression);
        public override void CallArgument(IValue function, Expression argumentExpression, ResolvedPort? port, IScope scope, Result<IValue> result) => PopResult(result);

        public override void BeforeExpressionBody(ExpressionBody expression, IScope scope) => Push("<call expression body>", expression.Expression);
        public override void ExpressionBody(ExpressionBody expression, IScope scope, Result<IValue> result) => PopResult(result);

        public override void BeforeScopeBody(FunctionBlock functionBlock, IScope scope) => Push("<call scope body>", functionBlock);

        public override void ScopeBody(FunctionBlock functionBlock, IScope scope, Result<IValue> result) => PopResult(result);

        /*
        public override void BeforeDefaultArgument(Port port, ExpressionBody? expressionBody, IScope scope)
        {
            if (expressionBody != null) Push($"<default argument> {expressionBody}", expressionBody.Expression);
        }
        public override Result<IValue> DefaultArgument(Port port, ExpressionBody? expressionBody, IScope scope, Result<IValue> result)
        {
            if (expressionBody != null) PopResult(result);
            return result;
        }*/
    }
}