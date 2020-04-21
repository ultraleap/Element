using System;
using System.Linq;

namespace Element.AST
{
    public sealed class ListType : IntrinsicType<ListType>
    {
        public enum CountType
        {
            Invalid,
            Constant,
            Dynamic,
        }
        
        public override string Name => "List";
        
        private static readonly Identifier _indexerId = new Identifier("at");
        private static readonly Identifier _countId = new Identifier("count");

        public override Port[] Inputs { get; } = {new Port(_indexerId, FunctionType.Instance), new Port(_countId, NumType.Instance)};

        public static (CountType CountType, int Count) GetListCount(StructInstance listInstance, CompilationContext context)
        {
            if (listInstance == null) throw new ArgumentNullException(nameof(listInstance));
            if (listInstance.Type != Instance)
            {
                context.LogError(8, "Struct instance is not a list");
                return (CountType.Invalid, -1);
            }

            return listInstance[_countId, false, context] switch
            {
                Constant c => (CountType.Constant, (int)c),
                Element.Expression expr => (CountType.Dynamic, -1), // Can't get count for a non-constant expression
                _ => context
                     .LogError(8, $"Couldn't get List.'{_countId}' from '{listInstance}'. Count must be a num expression.")
                     .Return((CountType.Invalid, -1))
            };
        }
        
        /// <summary>
        /// Converts an Element List instance to a fixed-size list of values by evaluating each index.
        /// </summary>
        /// <returns>The evaluated array, or an empty array if there was an error</returns>
        public static IValue[] EvaluateElements(StructInstance listInstance, CompilationContext context)
        {
            if (listInstance == null) throw new ArgumentNullException(nameof(listInstance));

            var (countType, count) = GetListCount(listInstance, context);

            switch (countType)
            {
                case CountType.Invalid: return Array.Empty<IValue>();
                case CountType.Constant:
                    if (!(listInstance[_indexerId, false, context] is IFunctionSignature indexer))
                    {
                        context.LogError(8, $"Couldn't get List.'{_indexerId}' from '{listInstance}'.");
                        return Array.Empty<IValue>();
                    }
            
                    return Enumerable.Range(0, count)
                                     .Select(i => indexer.ResolveCall(new IValue[] {new Constant(i)}, false, context))
                                     .ToArray();
                case CountType.Dynamic:
                    context.LogError(8, "Count must be a constant expression when performing fold.");
                    return Array.Empty<IValue>();
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }
    }
}