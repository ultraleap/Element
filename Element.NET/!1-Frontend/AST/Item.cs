using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    [WhitespaceSurrounded]
    public abstract class Item : IIdentifiable
    {
        public abstract Identifier Identifier { get; }
        public abstract bool Validate(CompilationContext compilationContext);

        public void Initialize(DeclaredScope parent)
        {
            Parent = parent ?? throw new ArgumentNullException(nameof(parent));
            // ReSharper disable once ConstantConditionalAccessQualifier
            Child?.Initialize(parent, this);
        }

        public DeclaredScope Parent { get; private set; }
        protected abstract DeclaredScope Child { get; }

        public string FullPath => $"{(Parent is GlobalScope ? string.Empty : $"{Parent.Identifier}.")}{Identifier}";

        static Item()
        {
            _intrinsics.Add("Any", AnyType.Instance);
            _intrinsics.Add("Num", NumType.Instance);


            /*
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
                _intrinsics.Add(fun.FullPath, fun);
            }
            //_functions.AddRange(Enum.GetValues(typeof(Unary.Op)).Cast<Unary.Op>().Select(o => new UnaryIntrinsic(o)));
        }

        private static readonly Dictionary<string, IValue> _intrinsics = new Dictionary<string, IValue>();

        public IValue? GetImplementingIntrinsic(CompilationContext compilationContext)
        {
            lock (_intrinsics)
            {
                return _intrinsics.TryGetValue(FullPath, out var implementingIntrinsic)
                    ? implementingIntrinsic
                    : compilationContext.LogError(4, $"Intrinsic '{FullPath}' not implemented");
            }
        }
    }
}