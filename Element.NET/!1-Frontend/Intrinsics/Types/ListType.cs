using System;
using System.Linq;

namespace Element.AST
{
    public sealed class ListType : IntrinsicType<ListType>
    {
        public override string Name => "List";
        
        private static readonly Identifier _indexerId = new Identifier("at");
        private static readonly Identifier _countId = new Identifier("count");

        public override Port[] Inputs { get; } = {new Port(_indexerId, FunctionType.Instance), new Port(_countId, NumType.Instance)};

        public static int? GetListCount(StructInstance listInstance, CompilationContext context)
        {
            if (listInstance == null) throw new ArgumentNullException(nameof(listInstance));
            if (listInstance.Type != Instance)
            {
                context.LogError(8, "Struct instance must be a list to evaluate elements.");
                return null;
            }
            
            if (!(listInstance[_countId, false, context] is Constant count))
            {
                context.LogError(8, $"Couldn't get List.'{_countId}' from '{listInstance}'. Count must be a constant to evaluate elements.");
                return null;
            }

            return (int)count;
        }
        
        /// <summary>
        /// Converts an Element List instance to a fixed-size list of values by evaluating each index.
        /// </summary>
        /// <returns>The evaluated array, or an empty array if there was an error</returns>
        public static IValue[] EvaluateElements(StructInstance listInstance, CompilationContext context)
        {
            if (listInstance == null) throw new ArgumentNullException(nameof(listInstance));

            var count = GetListCount(listInstance, context);
            if (!count.HasValue) return Array.Empty<IValue>();
            
            if (!(listInstance[_indexerId, false, context] is IFunctionSignature indexer))
            {
                context.LogError(8, $"Couldn't get List.'{_indexerId}' from '{listInstance}'.");
                return Array.Empty<IValue>();
            }
            
            return Enumerable.Range(0, count.Value)
                             .Select(i => indexer.ResolveCall(new IValue[] {new Constant(i)}, null, false, context))
                             .ToArray();
        }
    }
}