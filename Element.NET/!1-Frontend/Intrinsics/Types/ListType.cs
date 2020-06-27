using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class ListType : IntrinsicType
    {
        public static readonly Identifier IndexerId = new Identifier("at");
        public static readonly Identifier CountId = new Identifier("count");

        private ListType()
        {
            Identifier = new Identifier("List");
        }

        public static ListType Instance { get; } = new ListType();
        public override Identifier Identifier { get; }

        public override Result<IValue> Construct(StructDeclaration decl, IReadOnlyList<IValue> arguments, CompilationContext context) => new StructInstance(decl, arguments);
        public override Result<bool> MatchesConstraint(StructDeclaration decl, IValue value, CompilationContext context) => decl.IsInstanceOfStruct(value, context);
        public override Result<IValue> DefaultValue(CompilationContext context) => ListIntrinsic.Instance.Call(Array.Empty<IValue>(), context);

        public static Result<int> ConstantCount(StructInstance listInstance, CompilationContext context) =>
            listInstance.DeclaringStruct.IsIntrinsicType<ListType>()
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