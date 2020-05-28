using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public sealed class ListType : IIntrinsicType
    {
        private static readonly Identifier _indexerId = new Identifier("at");
        private static readonly Identifier _countId = new Identifier("count");

        private ListType()
        {
            Inputs = new[]{new Port(_indexerId, FunctionConstraint.Instance), new Port(_countId, NumType.Instance)};
            Output = Port.ReturnPort(AnyConstraint.Instance);
        }
        public static ListType Instance { get; } = new ListType();
        public Port[] Inputs { get; }
        public Port Output { get; }
        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            this.GetDeclaration(compilationContext).CreateInstance(arguments);
        public string Location => "List";
        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            value is StructInstance instance && instance.DeclaringStruct == this.GetDeclaration(compilationContext);
        public ISerializableValue DefaultValue(CompilationContext context) => MakeList(Array.Empty<IValue>(), context);
        IFunctionSignature IUnique<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this.GetDeclaration(compilationContext);
        public ISerializableValue MakeList(IValue[] elements, CompilationContext compilationContext) =>
            (ISerializableValue)this.ResolveCall(new IValue[]{new IndexFunction(elements), new Constant(elements.Length)}, false, compilationContext);

        private class IndexFunction : IFunction
        {
            private readonly IValue[] _elements;

            public IndexFunction(IValue[] elements) => _elements = elements;

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
                    Element.Expression indexExpr when elements.All(e => e is Element.Expression) => new Mux(indexExpr, elements.Cast<Element.Expression>()),
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

            public Port[] Inputs => _elements[0] is IFunctionSignature fn ? fn.Inputs : new[]{Port.VariadicPort};
            public Port Output => _elements[0] is IFunctionSignature fn ? fn.Output : Port.ReturnPort(AnyConstraint.Instance);
            public IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
                _elements[0] is IIndexable
                    ? Create(_index, _elements.Select(e => ((IIndexable)e)[id, recurse, compilationContext]).ToArray())
                    : compilationContext.LogError(16, "List element is not indexable");
        }

        public static bool HasConstantCount(StructInstance listInstance, out int constantCount, CompilationContext context)
        {
            if (listInstance == null) throw new ArgumentNullException(nameof(listInstance));
            if (listInstance.DeclaringStruct != Instance.GetDeclaration(context))
            {
                context.LogError(8, "Struct instance is not a list");
                constantCount = -1;
                return false;
            }

            constantCount = listInstance[_countId, false, context] switch
            {
                Constant c => (int)c,
                Element.Expression _ => -1, // Can't get count for a non-constant expression
                _ => context.LogError(8, $"Couldn't get List.'{_countId}' from '{listInstance}'. Count must be an expression.").Return(-1)
            };

            return constantCount >= 0;
        }

        public static IValue[] EvaluateElements(StructInstance listInstance, bool hasConstantCount, int count, CompilationContext compilationContext)
        {
            if (listInstance == null) throw new ArgumentNullException(nameof(listInstance));
            if (listInstance.DeclaringStruct != Instance.GetDeclaration(compilationContext))
            {
                compilationContext.LogError(8, "Struct instance is not a list");
                return Array.Empty<IValue>();
            }

            if (!hasConstantCount)
            {
                return new []{compilationContext.LogError(24, $"Cannot evaluate dynamic list '{listInstance}' at compile time")};
            }


            if (!(listInstance[_indexerId, false, compilationContext] is IFunctionSignature indexer))
            {
                compilationContext.LogError(8, $"Couldn't get List.'{_indexerId}' from '{listInstance}'.");
                return Array.Empty<IValue>();
            }
            
            return Enumerable.Range(0, count)
                             .Select(i => indexer.ResolveCall(new IValue[] {new Constant(i)}, false, compilationContext))
                             .ToArray();
        }
    }
}