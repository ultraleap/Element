using Element.AST;

namespace Element
{
    /// <summary>
    /// Caches compilation results for compilation of a scope, holding a reference to the frame of a parent scope.
    /// </summary>
    public class CompilationFrame
    {
        public CompilationFrame(Indexer indexer) :this(indexer, null) { }

        private CompilationFrame(Indexer indexer, CompilationFrame parent)
        {
            _indexer = indexer;
            _parent = parent;
        }

        private readonly CompilationFrame? _parent;
        private readonly Indexer _indexer;

        /// <summary>
        /// Push a child frame and return it.
        /// </summary>
        public CompilationFrame Push(Indexer indexer) => new CompilationFrame(indexer, this);

        /// <summary>
        /// Gets an item from this frame or a parent frame.
        /// </summary>
        public bool Get(Identifier identifier, CompilationContext context, out IValue value) =>
            GetLocal(identifier, context, out value) || (_parent?.Get(identifier, context, out value) ?? false);

        /// <summary>
        /// Gets an item from this frame excluding parent frames
        /// </summary>
        public bool GetLocal(Identifier identifier, CompilationContext context, out IValue value)
        {
            switch (_indexer?.Invoke(identifier, context))
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