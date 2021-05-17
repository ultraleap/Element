using System.Collections.Generic;
using System.Linq;
using ResultNET;

namespace Element.AST
{
    public static class IntrinsicImplementationCache
    {
        static IntrinsicImplementationCache()
        {
            foreach (var intrinsic in new IIntrinsicImplementation[]
                {
                    AnyConstraint.Instance,
                    NumStruct.Instance,
                    BoolStruct.Instance,
                    ListStruct.Instance,
                    For.Instance,
                    Fold.Instance,
                    List.Instance,
                    If.Instance
                }.Concat(Nullary.Instances.Values)
                 .Concat(Unary.Instances.Values)
                 .Concat(Binary.Instances.Values))
            {
                _intrinsics.Add(intrinsic.Identifier.String, intrinsic);
            }
        }

        public static Result<TIntrinsic> Get<TIntrinsic>(Identifier name, ITraceContext context)
            where TIntrinsic : IIntrinsicImplementation =>
            (_intrinsics.TryGetValue(name.String, out var intrinsic), intrinsic) switch
            {
                (true, TIntrinsic t) => t,
                (false, _) => context.Trace(EleMessageCode.IntrinsicNotFound, $"Intrinsic '{name}' is not implemented"),
                (true, _) => context.Trace(EleMessageCode.TypeError, $"Found intrinsic '{name}' but it is not '{typeof(TIntrinsic)}'")
            };

        private static readonly Dictionary<string, IIntrinsicImplementation> _intrinsics = new Dictionary<string, IIntrinsicImplementation>();
    }
}