namespace Element
{
    using System.Collections.Generic;

    /// <summary>
    /// Caches compilation results for compilation of a scope, holding a reference to the stack of a parent scope.
    /// </summary>
    public class CompilationStack
    {
        /// <summary>
        /// The stack frame a level above this one.
        /// </summary>
        private CompilationStack? Parent { get; set; }

        private readonly Dictionary<string, IFunction> _cache = new Dictionary<string, IFunction>();

        /// <summary>
        /// Creates a child stack frame.
        /// </summary>
        public CompilationStack Push() => new CompilationStack {Parent = this};

        /// <summary>
        /// Adds an item to the stack (e.g. an input or cached value)
        /// </summary>
        public void Add(string name, IFunction value) => _cache.Add(name, value);

        /// <summary>
        /// Gets an item from the stack (including parent frames)
        /// </summary>
        public bool Get(string name, out IFunction value) => _cache.TryGetValue(name, out value) || (Parent?.Get(name, out value) ?? false);

        /// <summary>
        /// Gets an item from this frame (i.e. excluding parent frames)
        /// </summary>
        public bool GetLocal(string name, out IFunction value) => _cache.TryGetValue(name, out value);
    }
}