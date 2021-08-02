using System;
using System.Collections.Generic;
using System.Linq;
using ResultNET;

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

        public Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, Context context) => StructInstance.Create(@struct, arguments, context).Cast<IValue>();
        public Result MatchesConstraint(Struct @struct, IValue value, Context context) =>
            @struct.IsInstanceOfStruct(value, context)
                ? Result.Success
                : context.Trace(ElementMessage.ConstraintNotSatisfied, $"Expected {Identifier} instance but got {value}");

        public Result<IValue> DefaultValue(Context context) => List.Instance.Call(Array.Empty<IValue>(), context);

        public static Result<int> ConstantCount(StructInstance listInstance, Context context) =>
            listInstance.DeclaringStruct.IsIntrinsicOfType<ListStruct>()
                ? listInstance.Index(CountId, context)
                              .Bind(countValue => countValue.InnerIs(out Constant constant)
                                   ? new Result<int>((int) constant)
                                   : countValue.InnerIs(out Instruction instruction)
                                       ? context.Trace(ElementMessage.NotCompileConstant, $"List count '{instruction}' is not a compile-time constant expression")
                                       : throw new InternalCompilerException($"Couldn't get List.'{CountId}' from '{listInstance}'. Count must be an expression."))
                : context.Trace(ElementMessage.TypeError, "Struct instance is not a List");

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