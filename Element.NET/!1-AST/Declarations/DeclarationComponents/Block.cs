using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded(ParserFlags = ParserFlags.IgnoreInTrace), MultiLine]
    public abstract class Block : AstNode
    {
#pragma warning disable 649, 169
        // ReSharper disable once UnusedAutoPropertyAccessor.Global
        // ReSharper disable once UnusedMember.Global
        protected abstract List<Declaration>? _items { get; set; }
#pragma warning restore 649, 169

        public IReadOnlyList<Declaration> Items => _items ?? (IReadOnlyList<Declaration>)Array.Empty<Declaration>();
        
        protected override void ValidateImpl(ResultBuilder builder, Context context)
        {
            var idHashSet = new HashSet<Identifier>();
            foreach (var decl in Items)
            {
                decl.Validate(builder, context);
                if (!idHashSet.Add(decl.Identifier))
                {
                    builder.Append(EleMessageCode.MultipleDefinitions, $"Multiple definitions for '{this}'");
                }
            }
            
            if (this is FunctionBlock && !idHashSet.Contains(Parser.ReturnIdentifier))
            {
                builder.Append(EleMessageCode.FunctionMissingReturn, $"Scope-bodied function '{this}' is missing return declaration");
            }
        }

        public Result<ResolvedBlock> ResolveBlock(IScope? parentScope, Context context, Func<IValue?>? valueProducedFrom = null) =>
            ResolveBlockWithCaptures(parentScope, Array.Empty<(Identifier Identifier, IValue Value)>(), context, valueProducedFrom);

        public Result<ResolvedBlock> ResolveBlockWithCaptures(IScope? parentScope,
                                                              IReadOnlyList<(Identifier Identifier, IValue Value)> capturedValues,
                                                              Context context,
                                                              Func<IValue?>? valueProducedFrom = null) =>
            Validate(context)
                .Map(() =>
                {
                    Result<IValue> IndexFunc(IScope scope, Identifier identifier, Context context) =>
                        Items.FirstOrDefault(d => d.Identifier.Equals(identifier))?.Resolve(scope, context)
                        ?? (Result<IValue>) context.Trace(EleMessageCode.IdentifierNotFound, $"'{identifier}' not found when indexing {scope}");

                    return new ResolvedBlock(Items.Select(d => d.Identifier).ToArray(),
                                             capturedValues,
                                             IndexFunc,
                                             parentScope,
                                             valueProducedFrom);
                });
    }

    public class FreeformBlock : Block
    {
        [
            SurroundBy("{", "}"),
            WhitespaceSurrounded,
            Optional
        ] protected override List<Declaration>? _items { get; set; }
    }

    public class CommaSeparatedBlock : Block
    {
        [
            SurroundBy("{", "}"),
            WhitespaceSurrounded,
            Optional,
            SeparatedBy(typeof(ListSeparator))
        ] protected override List<Declaration>? _items { get; set; }
    }

    public class DeclarationBlock : FreeformBlock, IDeclarationScope
    {
        public IReadOnlyList<Declaration> Declarations => Items;
    }
}