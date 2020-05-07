using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class ListType : SerializableIntrinsicType
    {
        private static readonly Identifier _indexerId = new Identifier("at");
        private static readonly Identifier _countId = new Identifier("count");
        
        public enum CountType
        {
            Invalid,
            Constant,
            Dynamic
        }
        
        public static ListType Instance { get; } = new ListType();
        protected override IntrinsicType _instance => Instance;
        
        public override string Name => "List";

        public override Port[] Inputs { get; } = {new Port(_indexerId, FunctionType.Instance), new Port(_countId, NumType.Instance)};
        public IValue MakeList(IValue[] elements, CompilationContext compilationContext) =>
            elements.All(e => e.Type == elements[0].Type)
                ? this.ResolveCall(new IValue[]{new IndexFunction(elements), new Constant(elements.Length)}, false, compilationContext)
                : compilationContext.LogError(8, "List elements must all be of the same type");

        public override int Size(IValue instance, CompilationContext compilationContext) =>
            GetListCount(instance as StructInstance, compilationContext) switch
            {
                (CountType t, int count) when t == CountType.Constant => count,
                _ => compilationContext.LogError(1, "List instance is not serializable - lists must have a constant count to be serializable")
                                       .Return(0)
            };

        public override bool Serialize(IValue instance, ref Element.Expression[] serialized, ref int position, CompilationContext compilationContext) =>
            instance is StructInstance listInstance
            && EvaluateElements(listInstance, compilationContext)
                .Serialize(ref serialized, ref position, compilationContext);

        public override IValue Deserialize(IEnumerable<Element.Expression> expressions, CompilationContext compilationContext) =>
            MakeList(expressions.ToArray(), compilationContext);

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
                ListElement.Create(arguments[0], _elements);
        }

        private class ListElement : IFunction, IIndexable
        {
            public static IValue Create(IValue index, IValue[] elements) =>
                index switch
                {
                    Element.Expression indexExpr when elements.All(e => e is Element.Expression) => (IValue) new Mux(indexExpr, elements.Cast<Element.Expression>()),
                    Constant constantIndex => elements[(int)constantIndex.Value],
                    _ => new ListElement(index, elements)
                };

            private ListElement(IValue index, IValue[] elements)
            {
                _index = index;
                _elements = elements;
            }

            IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;

            public override string ToString() => "<list element>";
            private readonly IValue _index;
            private readonly IValue[] _elements;
            
            IValue IFunction.Call(IValue[] arguments, CompilationContext context) =>
                _elements[0] is IFunctionSignature
                    ? Create(_index, _elements.Select(e => ((IFunctionSignature)e).ResolveCall(arguments, false, context)).ToArray())
                    : context.LogError(16, "List element is not a function - it cannot be called");

            public IType Type => _elements[0].Type;
            public Port[] Inputs => _elements[0] is IFunctionSignature fn ? fn.Inputs : new[]{Port.VariadicPort};
            public Port Output => _elements[0] is IFunctionSignature fn ? fn.Output : Port.ReturnPort(AnyConstraint.Instance);
            public IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
                _elements[0] is IIndexable
                    ? Create(_index, _elements.Select(e => ((IIndexable)e)[id, recurse, compilationContext]).ToArray())
                    : compilationContext.LogError(16, "List element is not indexable");
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
            if (listInstance.Type != Instance)
            {
                context.LogError(8, "Struct instance is not a list");
                return Array.Empty<IValue>();
            }

            var (countType, count) = GetListCount(listInstance, context);
            return EvaluateElements(listInstance, countType, count, context);
        }

        public static IValue[] EvaluateElements(StructInstance listInstance, CountType countType, int count, CompilationContext compilationContext)
        {
            if (listInstance == null) throw new ArgumentNullException(nameof(listInstance));
            if (listInstance.Type != Instance)
            {
                compilationContext.LogError(8, "Struct instance is not a list");
                return Array.Empty<IValue>();
            }
            
            switch (countType)
            {
                case CountType.Invalid:
                    throw new InternalCompilerException("List count type is invalid");
                case CountType.Constant:
                    if (!(listInstance[_indexerId, false, compilationContext] is IFunctionSignature indexer))
                    {
                        compilationContext.LogError(8, $"Couldn't get List.'{_indexerId}' from '{listInstance}'.");
                        return Array.Empty<IValue>();
                    }
            
                    return Enumerable.Range(0, count)
                                     .Select(i => indexer.ResolveCall(new IValue[] {new Constant(i)}, false, compilationContext))
                                     .ToArray();
                case CountType.Dynamic:
                    throw new InternalCompilerException("Cannot evaluate a dynamic lists elements at compile time");
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }
    }
}