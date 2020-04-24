using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public static class IntrinsicCache
    {
        static IntrinsicCache()
        {
            foreach (var intrinsic in new IIntrinsic[]
                {
                    AnyConstraint.Instance,
                    NumType.Instance,
                    BoolType.Instance,
                    ListType.Instance,
                    TupleType.Instance,
                    new ForIntrinsic(),
                    new FoldIntrinsic(),
                    new ListIntrinsic(),
                    new IfIntrinsic(),
                    new InferIntrinsic(),
                    new MemberwiseIntrinsic(),
                    new PersistIntrinsic()
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
                _intrinsics.Add(intrinsic.Location, intrinsic);
            }
        }

        public static TIntrinsic GetByLocation<TIntrinsic>(string location, Context? context)
            where TIntrinsic : class, IValue?
        {
            switch (_intrinsics.TryGetValue(location, out var intrinsic), intrinsic)
            {
                case (true, TIntrinsic t):
                    return t;
                case (false, _):
                    context?.LogError(4, $"Intrinsic '{location}' is not implemented");
                    return null;
                case (true, _):
                    context?.LogError(14, $"Found intrinsic '{location}' but it is not '{typeof(TIntrinsic)}'");
                    return null;
            }
        }

        private static readonly Dictionary<string, IValue> _intrinsics = new Dictionary<string, IValue>();
    }
}