using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Element.AST;

namespace Element
{
    public sealed class GlobalIndexer : IIndexable
    {
        private readonly Dictionary<FileInfo, SourceScope> _rootScopes = new Dictionary<FileInfo, SourceScope>();
        private readonly Dictionary<string, Item> _items = new Dictionary<string, Item>();
        private readonly Dictionary<string, IValue> _values = new Dictionary<string, IValue>();

        private readonly Dictionary<string, ICallable> _intrinsicFunctions = new Dictionary<string, ICallable>();

        public GlobalIndexer()
        {
            /*_types = new List<INamedType>
            {
                NumberType.Instance,
                AnyType.Instance
            };
            _functions = new List<INamedFunction>
            {
                new ArrayIntrinsic(),
                new FoldIntrinsic(),
                new MemberwiseIntrinsic(),
                new ForIntrinsic(),
                new PersistIntrinsic()
            };*/
            foreach (var fun in Enum.GetValues(typeof(Binary.Op))
                .Cast<Binary.Op>()
                .Select(o => new BinaryIntrinsic(o)))
            {
                _intrinsicFunctions.Add(fun.Name, fun);
            }
            //_functions.AddRange(Enum.GetValues(typeof(Unary.Op)).Cast<Unary.Op>().Select(o => new UnaryIntrinsic(o)));
        }

        public ICallable? GetIntrinsic(Identifier identifier) => _intrinsicFunctions.TryGetValue(identifier, out var callable) ? callable : null;

        public SourceScope this[FileInfo file]
        {
            get => _rootScopes[file];
            set => _rootScopes[file] = value;
        }

        public bool CanBeCached => true;
        public bool Validate(CompilationContext context) => context.ValidateScope(_rootScopes.Values.SelectMany(s => s), _items);

        public IValue? this[Identifier id, CompilationContext compilationContext] => compilationContext.Index(id, _items, _values);
    }
}