using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Element.AST;

namespace Element
{
    public sealed class GlobalIndexer : Scope
    {
        private readonly Dictionary<FileInfo, SourceScope> _rootScopes = new Dictionary<FileInfo, SourceScope>();

        private readonly Dictionary<Identifier, ICallable> _intrinsicFunctions = new Dictionary<Identifier, ICallable>();

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
                _intrinsicFunctions.Add(fun.Identifier, fun);
            }
            //_functions.AddRange(Enum.GetValues(typeof(Unary.Op)).Cast<Unary.Op>().Select(o => new UnaryIntrinsic(o)));
        }

        public ICallable? GetIntrinsic(Identifier identifier) => _intrinsicFunctions.TryGetValue(identifier, out var callable) ? callable : null;

        public SourceScope this[FileInfo file]
        {
            get => _rootScopes[file];
            set => _rootScopes[file] = value;
        }

        protected override IEnumerable<Item> ItemsToCacheOnValidate => _rootScopes.Values.SelectMany(s => s);
    }
}