using System;
using System.Collections.Generic;
using System.Globalization;
using Lexico;

#pragma warning disable 169,649

namespace Element.AST
{
    public interface IExpressionListStart {}
    public interface IFunctionBody {}

    public interface IValue
    {
        bool CanBeCached { get; }
    }
    public interface ICallable : IValue
    {
        IValue Call(Expression[] arguments, CompilationFrame frame, CompilationContext compilationContext);
    }

    public interface IIndexable
    {
        Item? this[Identifier id, CompilationContext compilationContext] { get; }
    }

    public delegate IValue? Indexer(Identifier identifier, CompilationContext compilationContext);

    public class Literal : IExpressionListStart, IValue
    {
        [field: Term]
        public float Value { get; }
        public static implicit operator float(Literal l) => l.Value;
        public override string ToString() => Value.ToString(CultureInfo.CurrentCulture);
        public bool CanBeCached => true;
    }

    public class Identifier : IExpressionListStart
    {
        public Identifier() {} // Need parameterless constructor for Lexico to construct instance
        public Identifier(string value) {Value = value;}

        // https://stackoverflow.com/questions/4400348/match-c-sharp-unicode-identifier-using-regex
        [field: Regex(@"[_\p{L}\p{Nl}][\p{L}\p{Nl}\p{Mn}\p{Mc}\p{Nd}\p{Pc}\p{Cf}]*")]
        public string Value { get; }
        public static implicit operator string(Identifier i) => i.Value;
        public override string ToString() => Value;
    }

    [WhitespaceSurrounded]
    struct ListSeparator
    {
        [Literal(",")] private Unnamed _;
        public override string ToString() => ",";
    }

    struct Terminal : IFunctionBody
    {
        [Literal(";")] private Unnamed _;
        public override string ToString() => ";";
    }

    [WhitespaceSurrounded]
    public abstract class ListOf<T>
    {
        [Literal("(")] private Unnamed _open;
        [field: SeparatedBy(typeof(ListSeparator))] public List<T> List { get; }
        [Literal(")")] private Unnamed _close;

        public override string ToString() => $"({string.Join(", ", List)})";
    }





    [WhitespaceSurrounded]
    public class Expression
    {
        [field: Term] public IExpressionListStart LitOrId { get; }
        [field: Optional] public List<CallExpression> CallExpressions { get; }

        public override string ToString() => $"{LitOrId}{(CallExpressions != null ? string.Concat(CallExpressions) : string.Empty)}";
    }

    public class CallExpression : ListOf<Expression> { } // CallExpression looks like a list due to using brackets

    public class PortList : ListOf<Port> { }

    [WhitespaceSurrounded]
    public class Port
    {
        [field: Term] public Identifier Identifier { get; }

        public override string ToString() => Identifier.ToString();
    }

    [WhitespaceSurrounded]
    public class Declaration
    {
        [field: Term] public Identifier Identifier { get; }
        [field: Optional] public PortList PortList { get; }

        public override string ToString() => $"{Identifier}{PortList}";
    }

    [WhitespaceSurrounded]
    public class Scope : IFunctionBody, IIndexable
    {
        [Literal("{")] private Unnamed _open;
        [Optional] private readonly List<Item> _items = new List<Item>();
        [Literal("}")] private Unnamed _close;

        private readonly Dictionary<string, Item> _cache = new Dictionary<string, Item>();

        public Item? this[Identifier id, CompilationContext compilationContext] => compilationContext.Index(id, _items, _cache);
    }

    [WhitespaceSurrounded]
    public class Binding : IFunctionBody
    {
        [Literal("=")] private Unnamed _bind;
        [field: Term] public Expression Expression { get; }
        [Term] private Terminal _terminal;

        public override string ToString() => $"= {Expression}";
    }



    public class Function : Item, ICallable
    {
        [Literal("intrinsic"), Optional] private Unnamed _;
        [Term] private Declaration _declaration;
        [Term] private IFunctionBody _functionBody;

        public override Identifier Identifier => _declaration.Identifier;
        public Port[] Inputs => _declaration.PortList?.List.ToArray();

        private static Identifier ReturnIdentifier { get; } = new Identifier("return");

        public IValue Call(Expression[] arguments, CompilationFrame frame, CompilationContext compilationContext)
        {
            var expectedArgCount = Inputs?.Length ?? 0; // No inputs means no arguments required (nullary function)
            if (arguments.Length != expectedArgCount)
            {
                compilationContext.LogError(6, $"Expected '{expectedArgCount}' arguments but got '{arguments.Length}'");
                return CompilationErr.Instance;
            }

            // TODO: Argument type checking

            var argumentsByIdentifier = new Dictionary<Identifier, Expression>();
            if (Inputs != null)
            {
                for (var i = 0; i < expectedArgCount; i++)
                {
                    argumentsByIdentifier.Add(Inputs[i].Identifier, arguments[i]);
                }
            }

            IValue? ArgumentIndexer(Identifier identifier, CompilationContext context) => compilationContext.CompileExpression(argumentsByIdentifier[identifier], frame);

            IValue CompileFunction(Function function, CompilationFrame functionFrame) =>
                function._functionBody switch
                {
                    Binding binding => compilationContext.CompileExpression(binding.Expression, functionFrame.Push(ArgumentIndexer)),
                    Scope scope => scope[ReturnIdentifier, compilationContext] switch
                    {
                        Function dependentFunction => CompileFunction(dependentFunction, functionFrame),
                        null => compilationContext.LogError(7, $"'{ReturnIdentifier}' not found in function scope"),
                        var nyi => throw new NotImplementedException(nyi.ToString())
                    },
                    _ => CompilationErr.Instance
                };

            return CompileFunction(this, frame);
        }

        public bool CanBeCached => Inputs == null || Inputs.Length == 0;
    }

    [WhitespaceSurrounded]
    public abstract class Item
    {
        public abstract Identifier Identifier { get; }
    }
}