using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class ListStruct : IIntrinsicStructImplementation
    {
        public static readonly Identifier IndexerId = new Identifier("at");
        public static readonly Identifier CountId = new Identifier("count");

        private ListStruct()
        {
            Identifier = new Identifier("List");
        }

        public static ListStruct Instance { get; } = new ListStruct();
        public Identifier Identifier { get; }

        public Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, Context context) => StructInstance.Create(@struct, arguments, context).Cast<IValue>(context);
        public Result<bool> MatchesConstraint(Struct @struct, IValue value, Context context) => @struct.IsInstanceOfStruct(value);
        public Result<IValue> DefaultValue(Context context) => List.Instance.Call(Array.Empty<IValue>(), context);

        public static Result<int> ConstantCount(StructInstance listInstance, Context context) =>
            listInstance.DeclaringStruct.IsIntrinsicOfType<ListStruct>()
                ? listInstance.Index(CountId, context)
                              .Bind(countValue => countValue switch
                              {
                                  Constant c => new Result<int>((int) c),
                                  Element.Instruction e => context.Trace(EleMessageCode.NotCompileConstant, $"List count '{e}' is not a compile-time constant expression"),
                                  _ => throw new InternalCompilerException($"Couldn't get List.'{CountId}' from '{listInstance}'. Count must be an expression.")
                              })
                : context.Trace(EleMessageCode.TypeError, "Struct instance is not a list");

        public static Result<IValue[]> EvaluateElements(StructInstance listInstance, Context context) =>
            ConstantCount(listInstance, context)
                .Accumulate(() => listInstance.Index(IndexerId, context))
                .Bind(tuple =>
                {
                    var (count, indexer) = tuple;
                    return Enumerable.Range(0, count)
                                     .Select(i => indexer.Call(new IValue[] {new Constant(i)}, context))
                                     .ToResultArray();
                });
    }
}