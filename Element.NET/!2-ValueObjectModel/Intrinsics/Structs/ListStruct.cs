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

        public Result<IValue> Construct(Struct @struct, IReadOnlyList<IValue> arguments, CompilationContext context) => new StructInstance(@struct, arguments);
        public Result<bool> MatchesConstraint(Struct @struct, IValue value, CompilationContext context) => @struct.IsInstanceOfStruct(value, context);
        public Result<IValue> DefaultValue(CompilationContext context) => List.Instance.Call(Array.Empty<IValue>(), context);

        public static Result<int> ConstantCount(StructInstance listInstance, CompilationContext context) =>
            listInstance.DeclaringStruct.IsIntrinsic<ListStruct>()
                ? listInstance.Index(CountId, context)
                              .Bind(countValue => countValue switch
                              {
                                  Constant c => new Result<int>((int) c),
                                  Element.Expression e => context.Trace(MessageCode.NotCompileConstant, $"List count '{e}' is not a compile-time constant expression"),
                                  _ => throw new InternalCompilerException($"Couldn't get List.'{CountId}' from '{listInstance}'. Count must be an expression.")
                              })
                : context.Trace(MessageCode.TypeError, "Struct instance is not a list");

        public static Result<IValue[]> EvaluateElements(StructInstance listInstance, CompilationContext context) =>
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