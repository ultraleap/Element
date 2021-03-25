using System;
using System.Collections.Generic;

namespace Element.AST
{
    /// <summary>
    /// A scope of declarations which can be resolved as a block.
    /// </summary>
    public interface IDeclarationScope
    {
        IReadOnlyList<Declaration> Declarations { get; }
        Result<ResolvedBlock> ResolveBlock(IScope? parentScope, Context context, Func<IValue>? valueProducedFrom = null);
    }
}