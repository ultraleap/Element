using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public static class IntrinsicCache
    {
        static IntrinsicCache()
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
        public static IValue? GetImplementingIntrinsic(this Item item) =>
            _intrinsics.TryGetValue(item.FullPath, out var intrinsic)
                ? intrinsic
                : null;
    }
}