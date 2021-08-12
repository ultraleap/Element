using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;
using ResultNET;

namespace Element
{
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