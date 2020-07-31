using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded, MultiLine]
    public abstract class Block : AstNode
    {
#pragma warning disable 649, 169
        // ReSharper disable once UnusedAutoPropertyAccessor.Global
        public abstract List<Declaration>? Items { get; protected set; }
#pragma warning restore 649, 169
        
        protected override void ValidateImpl(ResultBuilder builder, CompilationContext context)
        {
            var idHashSet = new HashSet<Identifier>();
            foreach (var decl in Items ?? Enumerable.Empty<Declaration>())
            {
                decl.Validate(builder, context);
                if (!idHashSet.Add(decl.Identifier))
                {
                    builder.Append(MessageCode.MultipleDefinitions, $"Multiple definitions for '{decl.Identifier}' defined in '{context.CurrentDeclarationLocation}'");
                }
            }
            
            if (this is FunctionBlock && !idHashSet.Contains(Parser.ReturnIdentifier))
            {
                builder.Append(MessageCode.FunctionMissingReturn, $"Scope-bodied function '{context.CurrentDeclarationLocation}' is missing return declaration");
            }
        }

        public Result<ResolvedBlock> ResolveBlock(IScope? parentScope, CompilationContext compilationContext) =>
            ResolveBlockWithCaptures(parentScope, Array.Empty<(Identifier Identifier, IValue Value)>(), compilationContext);

        public Result<ResolvedBlock> ResolveBlockWithCaptures(IScope? parentScope,
                                                              IReadOnlyList<(Identifier Identifier, IValue Value)> capturedValues,
                                                              CompilationContext compilationContext) =>
            Validate(compilationContext)
                .Map(() =>
                {
                    Result<IValue> IndexFunc(IScope scope, Identifier identifier, CompilationContext context) =>
                        Items.FirstOrDefault(d => d.Identifier.Equals(identifier))?.Resolve(scope, context)
                        ?? (Result<IValue>) context.Trace(MessageCode.IdentifierNotFound, $"'{identifier}' not found when indexing {scope}");

                    return new ResolvedBlock(null, Items?.Select(d => d.Identifier).ToArray() ?? Array.Empty<Identifier>(),
                                     capturedValues,
                                     IndexFunc,
                                     parentScope);
                });
    }

    public class FreeformBlock : Block
    {
        [field: SurroundBy("{", "}"), WhitespaceSurrounded, Optional] public override List<Declaration>? Items { get; protected set; }
    }

    public class CommaSeparatedBlock : Block
    {
        [field: SurroundBy("{", "}"), WhitespaceSurrounded, Optional, SeparatedBy(typeof(ListSeparator))] public override List<Declaration>? Items { get; protected set; }
    }

    public class DeclarationBlock : FreeformBlock, IDeclarationScope
    {
        public IReadOnlyList<Declaration> Declarations => Items as IReadOnlyList<Declaration> ?? Array.Empty<Declaration>();
    }
}