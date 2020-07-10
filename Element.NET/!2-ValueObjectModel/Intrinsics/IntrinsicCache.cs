using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public static class IntrinsicCache
    {
        static IntrinsicCache()
        {
            foreach (var intrinsic in new IntrinsicImplementation[]
                {
                    AnyConstraint.Instance,
                    NumStruct.Instance,
                    BoolStruct.Instance,
                    ListStruct.Instance,
                    TupleStruct.Instance,
                    ForIntrinsicFunctionImplementation.Instance,
                    FoldIntrinsicFunctionImplementation.Instance,
                    ListIntrinsicFunctionImplementation.Instance,
                    IfIntrinsicFunctionImplementation.Instance,
                    MemberwiseIntrinsicFunctionImplementation.Instance
                }.Concat(NullaryIntrinsicsFunctionImplementation.Instances.Values)
                 .Concat(UnaryIntrinsicFunctionImplementation.Instances.Values)
                 .Concat(BinaryIntrinsicFunctions.Instances.Values))
            {
                _intrinsics.Add(intrinsic.Identifier, intrinsic);
            }
        }

        public static Result<TIntrinsic> Get<TIntrinsic>(Identifier name, ITrace trace)
            where TIntrinsic : IntrinsicImplementation =>
            (_intrinsics.TryGetValue(name, out var intrinsic), intrinsic) switch
            {
                (true, TIntrinsic t) => t,
                (false, _) => trace.Trace(MessageCode.IntrinsicNotFound, $"Intrinsic '{name}' is not implemented"),
                (true, _) => trace.Trace(MessageCode.TypeError, $"Found intrinsic '{name}' but it is not '{typeof(TIntrinsic)}'")
            };

        private static readonly Dictionary<string, IntrinsicImplementation> _intrinsics = new Dictionary<string, IntrinsicImplementation>();
    }
}