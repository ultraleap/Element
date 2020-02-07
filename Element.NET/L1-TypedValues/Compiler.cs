using System;
using System.Collections.Generic;
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
                Literal lit => lit,
                Identifier id => frame.Get(id, compilationContext, out var value) ? value : CompilationErr.Instance,
                _ => CompilationErr.Instance
            };

            foreach (var expr in expression.CallExpressions)
            {
                if (previous is ICallable callable)
                {
                    previous = callable.Call(expr.List.ToArray(), frame, compilationContext);
                }
                else
                {
                    compilationContext.LogError(16, $"{previous} is not callable");
                }
            }
            //compilationContext.Pop();

            return previous;
        }

        public static Item? Index(this CompilationContext compilationContext, Identifier id, List<Item> items, Dictionary<string, Item> cache)
        {
            if (cache.TryGetValue(id, out var item)) return item;
            item = items.Find(i => string.Equals(i.Identifier, id, StringComparison.Ordinal));
            if (item != null)
            {
                cache[id] = item; // Don't cache item if it hasn't been found!
                compilationContext.LogError(7, $"'{id}' not found");
            }
            return item;
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
    }
}