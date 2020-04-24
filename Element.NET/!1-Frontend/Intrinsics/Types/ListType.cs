using System;
using System.Linq;

namespace Element.AST
{
    public sealed class ListType : IntrinsicType
    {
        private static readonly Identifier _indexerId = new Identifier("at");
        private static readonly Identifier _countId = new Identifier("count");
        
        public enum CountType
        {
            Invalid,
            Constant,
            Dynamic,
        }
        
        public static ListType Instance { get; } = new ListType();
        protected override IntrinsicType _instance => Instance;
        
        public override string Name => "List";

        public override Port[] Inputs { get; } = {new Port(_indexerId, FunctionType.Instance), new Port(_countId, NumType.Instance)};
        public IValue MakeList(IValue[] elements, CompilationContext compilationContext) =>
            elements.All(e => e.Type == elements[0].Type)
                ? this.ResolveCall(new IValue[]{new IndexFunction(elements), new Constant(elements.Length)}, false, compilationContext)
                : compilationContext.LogError(8, "List elements must all be of the same type");
        
        private class IndexFunction : IFunction
        {
            private readonly IValue[] _elements;

            public IndexFunction(IValue[] elements) => _elements = elements;

            IType IValue.Type => FunctionType.Instance;
            Port[] IFunctionSignature.Inputs { get; } = {new Port("i", NumType.Instance)};
            Port IFunctionSignature.Output { get; } = Port.ReturnPort(AnyConstraint.Instance);
            IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;

            public override string ToString() => "<list index function>";

            IValue IFunction.Call(IValue[] arguments, CompilationContext context) =>
                Indexer.Create(arguments[0], _elements);
        }

        private class Indexer : IFunction
        {
            public static IValue Create(IValue index, IValue[] elements) =>
                index is Element.Expression indexExpr && elements.All(e => e is Element.Expression)
                    ? (IValue) new Mux(indexExpr, elements.Cast<Element.Expression>())
                    : new Indexer(index, elements);

            private Indexer(IValue index, IValue[] elements)
            {
                _index = index;
                _elements = elements;
            }

            IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;

            public override string ToString() => "<list element>";
            private readonly IValue _index;
            private readonly IValue[] _elements;

            IValue IFunction.Call(IValue[] arguments, CompilationContext context) => Create(_index, _elements);

            public IType Type => FunctionType.Instance;
            public Port[] Inputs { get; } = Array.Empty<Port>();
            public Port Output { get; } = Port.ReturnPort(AnyConstraint.Instance);
        }

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
                Element.Expression _ => (CountType.Dynamic, -1), // Can't get count for a non-constant expression
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
                    // TODO: Should emit loop expression to perform fold at runtime
                    throw new NotImplementedException();
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }
    }
}