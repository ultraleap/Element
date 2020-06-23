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
        public override Port[] Fields { get; }
        public override Identifier Identifier { get; } = new Identifier("List");

        public override Result<IValue> Construct(StructDeclaration declaration, IEnumerable<IValue> arguments) => declaration.CreateInstance(arguments);
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            value is StructInstance instance
                ? Declaration(compilationContext.SourceContext)
                    .Map(decl => decl == instance.DeclaringStruct) 
                : false;
        public override Result<ISerializableValue> DefaultValue(CompilationContext context) => MakeList(Array.Empty<IValue>(), context).Map(v => (ISerializableValue)v);
        public Result<IValue> MakeList(IValue[] elements, CompilationContext context) =>
            context.SourceContext.GetIntrinsic<StructDeclaration>(Identifier)
                   .Bind(decl => Construct(decl, new IValue[]{new IndexFunction(elements), new Constant(elements.Length)}));

        private class IndexFunction : IFunction
        {
            private readonly IValue[] _elements;

            public IndexFunction(IValue[] elements) => _elements = elements;

            Port[] IFunctionSignature.Inputs { get; } = {new Port("i", NumType.Instance)};
            Port IFunctionSignature.Output { get; } = Port.ReturnPort(AnyConstraint.Instance);
            IFunctionSignature IInstancable<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;

            public override string ToString() => "<list index function>";

            Result<IValue> IFunction.Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
                new Result<IValue>(ListElement.Create(arguments[0], _elements));
        }

        private class ListElement : IFunction, IIndexable
        {
            public static IValue Create(IValue index, IReadOnlyList<IValue> elements) =>
                index switch
                {
                    Element.Expression indexExpr when elements.All(e => e is Element.Expression) => new Mux(indexExpr, elements.Cast<Element.Expression>()),
                    Constant constantIndex => elements[(int)constantIndex.Value],
                    _ => new ListElement(index, elements)
                };

            private ListElement(IValue index, IReadOnlyList<IValue> elements)
            {
                _index = index;
                _elements = elements;
            }

            IFunctionSignature IInstancable<IFunctionSignature>.GetDefinition(CompilationContext compilationContext) => this;

            public override string ToString() => "<list element>";
            private readonly IValue _index;
            private readonly IReadOnlyList<IValue> _elements;
            
            Result<IValue> IFunction.Call(IReadOnlyList<IValue> arguments, CompilationContext context) =>
                _elements[0] is IFunctionSignature
                    ? _elements.Select(e => ((IFunctionSignature)e).ResolveCall(arguments.ToArray(), false, context))
                                                                   .MapEnumerable(v => Create(_index, v.ToList()))
                    : context.Trace(MessageCode.InvalidExpression, "List element is not a function - it cannot be called");

            public Port[] Inputs => _elements[0] is IFunctionSignature fn ? fn.Inputs : new[]{Port.VariadicPort};
            public Port Output => _elements[0] is IFunctionSignature fn ? fn.Output : Port.ReturnPort(AnyConstraint.Instance);
            public Result<IValue> this[Identifier id, bool recurse, CompilationContext context] =>
                _elements[0] is IIndexable
                    ? Create(_index, _elements.Select(e => ((IIndexable)e)[id, recurse, context]).ToList())
                    : context.Trace(MessageCode.InvalidExpression, "List element is not indexable");
        }

        public static bool HasConstantCount(StructInstance listInstance, out int constantCount, CompilationContext context)
        {
            if (listInstance.DeclaringStruct != Instance.Declaration(context))
            {
                context.Flush(8, "Struct instance is not a list");
                constantCount = -1;
                return false;
            }

            constantCount = listInstance[_countId, false, context] switch
            {
                Constant c => (int)c,
                Element.Expression _ => -1, // Can't get count for a non-constant expression
                _ => context.Flush(8, $"Couldn't get List.'{_countId}' from '{listInstance}'. Count must be an expression.").Return(-1)
            };

            return constantCount >= 0;
        }

        public static IValue[] EvaluateElements(StructInstance listInstance, bool hasConstantCount, int count, CompilationContext compilationContext)
        {
            if (listInstance == null) throw new ArgumentNullException(nameof(listInstance));
            if (listInstance.DeclaringStruct != Instance.GetDeclaration(compilationContext))
            {
                compilationContext.Flush(8, "Struct instance is not a list");
                return Array.Empty<IValue>();
            }

            if (!hasConstantCount)
            {
                return new []{compilationContext.Flush(24, $"Cannot evaluate dynamic list '{listInstance}' at compile time")};
            }


            if (!(listInstance[_indexerId, false, TODO] is IFunctionSignature indexer))
            {
                compilationContext.Flush(8, $"Couldn't get List.'{_indexerId}' from '{listInstance}'.");
                return Array.Empty<IValue>();
            }
            
            return Enumerable.Range(0, count)
                             .Select(i => indexer.ResolveCall(new IValue[] {new Constant(i)}, false, compilationContext))
                             .ToArray();
        }
    }
}