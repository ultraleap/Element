using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;

namespace Element
{
    public static class Compiler
    {
        private sealed class FunctionArguments : IIndexable
        {
            public FunctionArguments(IValue[] arguments, Port[] ports)
            {
                for (var i = 0; i < ports?.Length; i++)
                {
                    _argumentsByIdentifier.Add(ports[i].Identifier, arguments[i]);
                }
            }

            private readonly Dictionary<Identifier, IValue> _argumentsByIdentifier = new Dictionary<Identifier, IValue>();

            public bool CanBeCached => true;

            public IValue? this[Identifier id] => _argumentsByIdentifier.TryGetValue(id, out var arg) ? arg : null;
        }
        
        public static IValue GetArgumentByIndex(this ICallable callable, int index, CompilationFrame frame, CompilationContext context)
        {
            if (callable.Inputs == null) throw new InternalCompilerException($"{callable} does not have inputs");
            var identifier = callable.Inputs[index].Identifier;
            return frame.Get(identifier, context, out var value) ? value : context.LogError(7, $"Couldn't find '{identifier}'");
        }
        
        public static IValue CompileExpression(this CompilationContext compilationContext, AST.Expression expression, CompilationFrame frame)
        {
            var previous = expression.LitOrId switch
            {
                // If the start of the list is an identifier, find the value that it identifies
                Identifier id => frame.Get(id, compilationContext, out var value) ? value : compilationContext.LogError(7, $"Couldn't find '{id}' in a local or outer scope"),
                Literal lit => lit,
                _ => throw new InternalCompilerException("Trying to compile expression that doesn't start with literal or identifier - should be impossible")
            };
            
            // Early out if something failed above
            if (previous is CompilationErr) return CompilationErr.Instance;
            
            compilationContext.Push(new TraceSite(previous.ToString(), null, 0, 0));

            // Resolve Nullary (0-argument) functions
            previous = previous switch
            {
                Function func when func.Inputs.Length == 0 => func.Call(frame, compilationContext),
                _ => previous
            };

            // TODO: Handle lambdas
            
            // TODO: When adding indexing, generalize to resolving all expression types
            foreach (var callExpr in expression.CallExpressions)
            {
                if (!(previous is ICallable callable)) return compilationContext.LogError(16, $"{previous} is not callable");
                
                // Compile the arguments for this call expression
                var arguments = callExpr.List.Select(argExpr => compilationContext.CompileExpression(argExpr, frame)).ToArray();
                
                // Check argument count is correct
                var expectedArgCount = callable.Inputs?.Length ?? 0; // No inputs means no arguments required (nullary function)
                if (arguments.Length != expectedArgCount)
                {
                    return compilationContext.LogError(6, $"Expected '{expectedArgCount}' arguments but got '{arguments.Length}'"); 
                }

                // TODO: Argument type checking
                
                // Wraps arguments in a FunctionArguments compilation frame for the called function to access
                IValue Call(ICallable c) => c.Call(frame.Push(new FunctionArguments(arguments, c.Inputs)), compilationContext);

                // 
                previous = callable switch
                {
                    Function intrinsic when intrinsic.IsIntrinsic => Call(compilationContext.GlobalIndexer.GetIntrinsic(intrinsic.Identifier)),
                    Function f => Call(f),
                    _ => throw new NotImplementedException($"Calling '{callable}' not yet implemented in CompileExpression")
                };
            }
            compilationContext.Pop();

            return previous;
        }
    }
}