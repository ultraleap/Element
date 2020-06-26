using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class ListType : IntrinsicType
    {
        private static readonly Identifier _indexerId = new Identifier("at");
        private static readonly Identifier _countId = new Identifier("count");

        private ListType() => Fields = new[]{new Port(_indexerId, FunctionConstraint.Instance), new Port(_countId, NumType.Instance)};
        public static ListType Instance { get; } = new ListType();
        public override IReadOnlyList<Port> Fields { get; }
        public override Identifier Identifier { get; } = new Identifier("List");

        public override Result<IValue> Construct(IReadOnlyList<IValue> arguments, CompilationContext context) => Declaration(context.SourceContext).Map(decl => (IValue)new StructInstance(decl, arguments));
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => Declaration(context.SourceContext).Bind(decl => decl.IsInstanceOfStruct(value, context));
        public override Result<ISerializableValue> DefaultValue(CompilationContext context) => ListIntrinsic.Instance.Call(Array.Empty<IValue>(), context).Cast<ISerializableValue>(context);
        
        public static Result<int> ConstantCount(StructInstance listInstance, CompilationContext context) =>
            Instance.Declaration(context.SourceContext)
                    .Check(listTypeDecl =>
                               listInstance.DeclaringStruct != listTypeDecl
                                   ? context.Trace(MessageCode.TypeError, "Struct instance is not a list")
                                   : Result.Success)
                    .Bind(() => listInstance[_countId, false, context]
                              .Bind(countValue => countValue switch
                              {
                                  Constant c => new Result<int>((int) c),
                                  Element.Expression e => context.Trace(MessageCode.NotCompileConstant, $"List count '{e}' is not a compile-time constant expression"),
                                  _ => throw new InternalCompilerException($"Couldn't get List.'{_countId}' from '{listInstance}'. Count must be an expression.")
                              }));

        public static Result<IValue[]> EvaluateElements(StructInstance listInstance, CompilationContext context) =>
            ConstantCount(listInstance, context)
                .Accumulate(() => listInstance[_indexerId, false, context].Cast<IFunction>(() => throw new InternalCompilerException($"Couldn't get List.'{_indexerId}' from '{listInstance}'. Indexer must be a function.")))
                .Bind(tuple =>
                {
                    var (count, indexer) = tuple;
                    return Enumerable.Range(0, count)
                                     .Select(i => indexer.Call(new IValue[] {new Constant(i)}, context))
                                     .ToResultArray();
                });
    }
}