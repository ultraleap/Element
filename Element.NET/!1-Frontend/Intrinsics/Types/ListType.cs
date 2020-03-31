using System;
using System.Linq;

namespace Element.AST
{
    public sealed class ListType : IntrinsicType<ListType>
    {
        public override string Name => "List";
        public override ISerializer? Serializer { get; } = new ListSerializer();

        private class ListSerializer : ISerializer
        {
            public int SerializedSize(IValue value) =>
                value is StructInstance instance
                && instance.Type == Instance
                    ? GetCount(instance)
                    : 0;

            public bool Serialize(IValue value, ref float[] array, ref int position)
            {
                if (!(value is StructInstance instance) || instance.Type != Instance) return false;
                var success = true;
                foreach (var element in EvaluateElements(instance, null) ?? Enumerable.Empty<IValue>())
                {
                    success &= element.Type.Serializer.Serialize(element, ref array, ref position);
                }

                return success;
            }

            private static int GetCount(StructInstance listInstance) => Convert.ToInt32((Literal) listInstance[_countId, false, null]);
        }
        
        private static readonly Identifier _indexerId = new Identifier("at");
        private static readonly Identifier _countId = new Identifier("count");

        public override Port[] Inputs { get; } = {new Port(_indexerId, FunctionType.Instance), new Port(_countId, NumType.Instance)};
        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            this.GetDeclaration<DeclaredStruct>(compilationContext)?.CreateInstance(arguments, Instance);
        
        /// <summary>
        /// Converts an Element List instance to a fixed-size list of values by evaluating each index.
        /// </summary>
        /// <returns>The evaluated array, or an empty array if there was an error</returns>
        public static IValue[]? EvaluateElements(IScope listInstance, CompilationContext context)
        {
            if (listInstance == null) throw new ArgumentNullException(nameof(listInstance));

            if (!(listInstance[_indexerId, false, context] is ICallable indexer))
            {
                context.LogError(8, $"Couldn't get List.'{_indexerId}' from '{listInstance}'. Is '{listInstance}' a List?");
                return null;
            }

            if (!(listInstance[_countId, false, context] is Literal count))
            {
                context.LogError(8, $"Couldn't get List.'{_countId}' from '{listInstance}'. Is '{listInstance}' a List?");
                return null;
            }

            /*var countExpr = listInstance.Call("count", context).AsExpression(context);
            if (countExpr != null) { countExpr = ConstantFolding.Optimize(countExpr); }

            var count = (countExpr as Constant)?.Value;
            if (!count.HasValue)
            {
                context.LogError(9999, $"{listInstance}'s count is not constant");
                return Array.Empty<IFunction>();
            }*/

            return Enumerable.Range(0, (int)count.Value)
                             .Select(i => indexer.Call(new IValue[] {new Literal(i)}, context))
                             .ToArray();
        }
    }
}