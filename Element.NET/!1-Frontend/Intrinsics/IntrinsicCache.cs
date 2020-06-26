using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public static class IntrinsicCache
    {
        static IntrinsicCache()
        {
            foreach (var intrinsic in new Intrinsic[]
                {
                    AnyConstraint.Instance,
                    NumType.Instance,
                    BoolType.Instance,
                    ListType.Instance,
                    TupleType.Instance,
                    ForIntrinsic.Instance,
                    FoldIntrinsic.Instance,
                    ListIntrinsic.Instance,
                    IfIntrinsic.Instance,
                    MemberwiseIntrinsic.Instance
                }.Concat(Enum.GetValues(typeof(Constant.Intrinsic))
                             .Cast<Constant.Intrinsic>()
                             .Select(v => new NullaryIntrinsics(v)))
                 .Concat(Enum.GetValues(typeof(Unary.Op))
                             .Cast<Unary.Op>()
                             .Select(o => new UnaryIntrinsic(o)))
                 .Concat(Enum.GetValues(typeof(Binary.Op))
                             .Cast<Binary.Op>()
                             .Select(o => new BinaryIntrinsic(o))))
            {
                _intrinsics.Add(intrinsic.Identifier, intrinsic);
            }
        }

        public static Result<TIntrinsic> Get<TIntrinsic>(Identifier name, ITrace? trace)
            where TIntrinsic : Intrinsic =>
            (_intrinsics.TryGetValue(name, out var intrinsic), intrinsic) switch
            {
                (true, TIntrinsic t) => t,
                (false, _) => trace?.Trace(MessageCode.IntrinsicNotFound, $"Intrinsic '{name}' is not implemented"),
                (true, _) => trace?.Trace(MessageCode.TypeError, $"Found intrinsic '{name}' but it is not '{typeof(TIntrinsic)}'")
            };

        private static readonly Dictionary<string, Intrinsic> _intrinsics = new Dictionary<string, Intrinsic>();
    }
}