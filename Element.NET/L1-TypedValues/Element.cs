using System;
using System.Collections.Generic;
using System.Globalization;
using Lexico;

#pragma warning disable 169,649

namespace Element.AST
{
    public interface IExpressionListStart {}
    public interface IFunctionBody {}
    public interface IValue {}
    public interface ICallable
    {
        IValue Call(in IValue[] arguments);
    }

    public delegate Item? Indexer(Identifier identifier, CompilationContext compilationContext);

    public class Literal : IExpressionListStart, IValue
    {
        [field: Term]
        public float Value { get; }
        public static implicit operator float(Literal l) => l.Value;
        public override string ToString() => Value.ToString(CultureInfo.CurrentCulture);
    }

    public class Identifier : IExpressionListStart
    {
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
    }

    struct Terminal : IFunctionBody
    {
        [Literal(";")] private Unnamed _;
    }

    [WhitespaceSurrounded]
    public abstract class ListOf<T>
    {
        [Literal("(")] private Unnamed _open;
        [field: SeparatedBy(typeof(ListSeparator))] public List<T> List { get; }
        [Literal(")")] private Unnamed _close;
    }





    [WhitespaceSurrounded]
    public class Expression
    {
        [field: Term] public IExpressionListStart LitOrId { get; }
        [field: Optional] public List<CallExpression> CallExpressions { get; }
    }

    public class CallExpression : ListOf<Expression> // CallExpression looks like a list due to using brackets
    {
    }

    public class PortList : ListOf<Port>
    {
    }

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

        public override string ToString() => Identifier.Value;
    }

    [WhitespaceSurrounded]
    public class Scope : IFunctionBody
    {
        [Literal("{")] private Unnamed _open;
        [Optional] private List<Item> _items;
        [Literal("}")] private Unnamed _close;

        private readonly Dictionary<string, Item> _cache = new Dictionary<string, Item>();

        public Item? this[Identifier id, CompilationContext compilationContext]
        {
            get
            {
                if (_cache.TryGetValue(id, out var item)) return item;
                item = _items.Find(i => string.Equals(i.Identifier, id, StringComparison.Ordinal));
                if (item != null)
                {
                    _cache[id] = item; // Don't cache item if it hasn't been found!
                    compilationContext.LogError(7, $"'{id}' not found in {this}");
                }
                return item;
            }
        }
    }

    [WhitespaceSurrounded]
    public class Binding : IFunctionBody
    {
        [Literal("=")] private Unnamed _bind;
        [Term] private Expression _expression;
        [Term] private Terminal _terminal;
    }



    public class Function : Item, ICallable
    {
        [Literal("intrinsic"), Optional] private Unnamed _;
        [Term] private Declaration _declaration;
        [Term] private IFunctionBody _functionBody;

        public override Identifier Identifier => _declaration.Identifier;

        public IValue Call(in IValue[] arguments) => _functionBody switch
        {
            Literal lit => lit,
            _ => CompilationErr.Instance
        };
    }

    [WhitespaceSurrounded]
    public abstract class Item
    {
        public abstract Identifier Identifier { get; }
    }
}