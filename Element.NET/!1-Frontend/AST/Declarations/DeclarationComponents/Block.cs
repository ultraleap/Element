using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    public abstract class BlockBase : AstNode
    {
#pragma warning disable 649, 169
        // ReSharper disable once UnusedAutoPropertyAccessor.Local
        [SurroundBy("{", "}"), WhitespaceSurrounded, Optional] public List<Declaration>? Items { get; private set; }
#pragma warning restore 649, 169
        
        protected abstract bool IsFunctionBlock { get; }

        protected override void ValidateImpl(ResultBuilder resultBuilder, CompilationContext context)
        {
            var idHashSet = new HashSet<Identifier>();
            foreach (var decl in Items ?? Enumerable.Empty<Declaration>())
            {
                decl.Identifier.Validate(resultBuilder,
                                         Array.Empty<Identifier>(),
                                         IsFunctionBlock
                                             ? new []{Parser.ReturnIdentifier}
                                             : Array.Empty<Identifier>());
                decl.Validate(resultBuilder, context);
                if (!idHashSet.Add(decl.Identifier))
                {
                    resultBuilder.Append(MessageCode.MultipleDefinitions, $"Multiple definitions for '{decl.Identifier}' defined in '{context.CurrentDeclarationLocation}'");
                }
            }

            if (!idHashSet.Contains(Parser.ReturnIdentifier))
            {
                resultBuilder.Append(MessageCode.FunctionMissingReturn, $"Scope-bodied function '{context.CurrentDeclarationLocation}' is missing Return declaration");
            }
        }

        public Result<IScope> Resolve(IScope? parentScope, CompilationContext compilationContext) =>
            Validate(compilationContext)
                .Map(() =>
                {
                    var indexingCache = new Dictionary<Identifier, Result<IValue>>();

                    Result<IValue> IndexFunc(IScope scope, Identifier identifier, CompilationContext context)
                    {
                        if (indexingCache.TryGetValue(identifier, out var result)) return result;
                        result = Items.FirstOrDefault(d => d.Identifier == identifier)?.Resolve(scope, context)
                                 ?? (Result<IValue>) context.Trace(MessageCode.IdentifierNotFound, $"'{identifier}' not found in '{context.CurrentDeclarationLocation}'");
                        return indexingCache[identifier] = result;
                    }

                    return (IScope) new Scope(Items?.Select(d => d.Identifier).ToArray() ?? Array.Empty<Identifier>(),
                                              IndexFunc,
                                              parentScope);
                });
    }

    public class FunctionBlock : BlockBase
    {
        protected override bool IsFunctionBlock => true;
    }

    public class Block : BlockBase, IEnumerable<Declaration>
    {
        protected override bool IsFunctionBlock => false;
        public IEnumerator<Declaration> GetEnumerator() => Items?.GetEnumerator() ?? Enumerable.Empty<Declaration>().GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
    }
}