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
        
        protected override void ValidateImpl(ResultBuilder builder, Context context)
        {
            var idHashSet = new HashSet<Identifier>();
            foreach (var decl in Items ?? Enumerable.Empty<Declaration>())
            {
                decl.Validate(builder, context);
                if (!idHashSet.Add(decl.Identifier))
                {
                    builder.Append(MessageCode.MultipleDefinitions, $"Multiple definitions for '{this}'");
                }
            }
            
            if (this is FunctionBlock && !idHashSet.Contains(Parser.ReturnIdentifier))
            {
                builder.Append(MessageCode.FunctionMissingReturn, $"Scope-bodied function '{this}' is missing return declaration");
            }
        }

        public Result<ResolvedBlock> ResolveBlock(IScope? parentScope, Context context) =>
            ResolveBlockWithCaptures(parentScope, Array.Empty<(Identifier Identifier, IValue Value)>(), context);

        public Result<ResolvedBlock> ResolveBlockWithCaptures(IScope? parentScope,
                                                              IReadOnlyList<(Identifier Identifier, IValue Value)> capturedValues,
                                                              Context context) =>
            Validate(context)
                .Map(() =>
                {
                    Result<IValue> IndexFunc(IScope scope, Identifier identifier, Context context) =>
                        Items.FirstOrDefault(d => d.Identifier.Equals(identifier))?.Resolve(scope, context)
                        ?? (Result<IValue>) context.Trace(MessageCode.IdentifierNotFound, $"'{identifier}' not found when indexing {scope}");

                    return new ResolvedBlock(Items?.Select(d => d.Identifier).ToArray() ?? Array.Empty<Identifier>(),
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