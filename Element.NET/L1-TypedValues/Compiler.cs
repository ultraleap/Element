using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;

namespace Element
{
    public static class Compiler
    {
        public static IValue CompileExpression(this CompilationContext compilationContext, AST.Expression expression, CompilationFrame frame)
        {
            //compilationContext.Push();
            var previous = expression.LitOrId switch
            {
                Identifier id => frame.Get(id, compilationContext, out var value) ? value : CompilationErr.Instance,
                Literal lit => lit,
                _ => CompilationErr.Instance
            };

            // Resolve Nullary functions
            previous = previous switch
            {
                Function func when func.Inputs.Length == 0 => func.Call(null, frame, compilationContext),
                _ => previous
            };

            foreach (var callExpr in expression.CallExpressions)
            {
                var arguments = callExpr.List.Select(argExpr => compilationContext.CompileExpression(argExpr, frame)).ToArray();

                IValue Call(ICallable callable) =>
                    callable.Call(arguments, frame.Push(new Function.FunctionArguments(arguments, callable.Inputs)),
                                  compilationContext);

                previous = previous switch
                {
                    ICallable callable => callable switch
                    {
                        Function function => frame.Get(function.Identifier, compilationContext, out var functionDeclaration) switch
                            {
                                true => functionDeclaration switch
                                {
                                    Function intrinsic when intrinsic.IsIntrinsic => Call(compilationContext.GlobalIndexer.GetIntrinsic(intrinsic.Identifier)),
                                    Function f => Call(f),
                                    _ => compilationContext.LogError(9999, $"'{functionDeclaration}' is not a function")
                                },
                                false => compilationContext.LogError(7, $"Couldn't find function declaration '{function.Identifier}'")
                            },
                        _ => Call(callable)
                    },
                    _ => compilationContext.LogError(16, $"{previous} is not callable")
                };
            }
            //compilationContext.Pop();

            return previous;
        }

        public static bool CheckArguments(this CompilationContext compilationContext, IValue[] arguments, Port[] inputs)
        {
            var argCount = arguments?.Length ?? 0;
            var expectedArgCount = inputs?.Length ?? 0; // No inputs means no arguments required (nullary function)
            if (argCount != expectedArgCount)
            {
                compilationContext.LogError(6, $"Expected '{expectedArgCount}' arguments but got '{argCount}'");
                return false;
            }

            // TODO: Argument type checking

            return true;
        }
    }
}