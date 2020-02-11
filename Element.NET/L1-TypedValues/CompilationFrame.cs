using Element.AST;

namespace Element
{
    /// <summary>
    /// Caches compilation results for compilation of a scope, holding a reference to the frame of a parent scope.
    /// </summary>
    public class CompilationFrame
    {
        public CompilationFrame(IIndexable indexable) :this(indexable, null) { }

        private CompilationFrame(IIndexable indexable, CompilationFrame parent)
        {
            _indexable = indexable;
            _parent = parent;
        }

        private readonly CompilationFrame? _parent;
        private readonly IIndexable _indexable;

        /// <summary>
        /// Push a child frame and return it.
        /// </summary>
        public CompilationFrame Push(IIndexable indexer) => new CompilationFrame(indexer, this);

        /// <summary>
        /// Gets an item from this frame or a parent frame.
        /// </summary>
        public bool Get(Identifier identifier, CompilationContext context, out IValue? value) =>
            GetLocal(identifier, context,  out value) || (_parent?.Get(identifier, context, out value) ?? false);

        /// <summary>
        /// Gets an item from this frame excluding parent frames
        /// </summary>
        public bool GetLocal(Identifier identifier, CompilationContext context, out IValue? value)
        {
            switch (_indexable[identifier, context])
            {
                case { } i:
                    value = i;
                    return true;
                default:
                    value = default;
                    return false;
            }
        }
    }
}