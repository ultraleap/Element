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
                var arguments = callExpr.List.Select(argExpr =>
                    (Func<IValue>) (() => compilationContext.CompileExpression(argExpr, frame))).ToArray();

                IValue Call(ICallable callable)
                {
                    return callable.Call(arguments, frame.Push(new Function.FunctionArguments(arguments, callable.Inputs)),
                        compilationContext);
                }

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

        public static IValue? Index(this CompilationContext compilationContext, Identifier id, Dictionary<string, Item> items, Dictionary<string, IValue> cache)
        {
            if (cache.TryGetValue(id, out var value)) return value;
            value = items.TryGetValue(id, out var item) switch
            {
                true => item switch
                {
                    IValue v => v,
                    _ => throw new InternalCompilerException($"{item} is not an IValue")
                },
                false => compilationContext.LogError(7, $"'{id}' not found")
            };
            if (value != null && value.CanBeCached)
            {
                cache[id] = value;
            }

            return value;
        }

        public static bool ValidateScope(this CompilationContext compilationContext, IEnumerable<Item> items, Dictionary<string, Item> cache)
        {
            var success = true;
            foreach (var item in items)
            {
                if (!compilationContext.ValidateIdentifier(item.Identifier))
                {
                    success = false;
                    continue;
                }

                if (cache.ContainsKey(item.Identifier))
                {
                    compilationContext.LogError(2, $"Cannot add duplicate identifier '{item.Identifier}'");
                    success = false;
                }
                else
                {
                    cache[item.Identifier] = item;
                }
            }

            return success;
        }

        public static bool CheckArguments(this CompilationContext compilationContext, Func<IValue>[] arguments, Port[] inputs)
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