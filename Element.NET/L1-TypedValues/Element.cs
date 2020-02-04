using System.Collections.Generic;
using System.Globalization;
using Lexico;

#pragma warning disable 169,649

namespace Element.AST
{
    public interface IExpressionListStart {}
    public interface IFunctionBody {}
    public interface IValue {}
    public interface ICallable {}

    public class Literal : IExpressionListStart, IValue
    {
        [SequenceTerm] private float _value;
        public static implicit operator float(Literal l) => l._value;
        public float Value => _value;

        public override string ToString() => _value.ToString(CultureInfo.CurrentCulture);
    }

    public class Identifier : IExpressionListStart
    {
        [SequenceTerm, Regex(@"[_\p{L}\p{Nl}][\p{L}\p{Nl}\p{Mn}\p{Mc}\p{Nd}\p{Pc}\p{Cf}]*")] private string _value;

        public static implicit operator string(Identifier i) => i._value;
        public string Value => _value;

        public override string ToString() => _value;
    }

    [WhitespaceSurrounded]
    struct ListSeparator
    {
        [SequenceTerm, Literal(",")] private Unnamed _;
    }

    struct Terminal : IFunctionBody
    {
        [SequenceTerm, Literal(";")] private Unnamed _;
    }

    [WhitespaceSurrounded]
    public abstract class ListOf<T>
    {
        [SequenceTerm, Literal("(")] private Unnamed _open;
        [SequenceTerm, SeparatedBy(typeof(ListSeparator))] protected List<T> _list;
        [SequenceTerm, Literal(")")] private Unnamed _close;
    }





    [WhitespaceSurrounded]
    public class Expression
    {
        [SequenceTerm] private IExpressionListStart _start;
        [SequenceTerm, Optional] private List<CallExpression> _list;
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
        [SequenceTerm] private Identifier _portName;

        public string PortName => _portName.Value;

        public override string ToString() => _portName.ToString();
    }

    [WhitespaceSurrounded]
    public class Declaration
    {
        [SequenceTerm] private Identifier _identifier;
        [SequenceTerm, Optional] private PortList _portList;

        public string Name => _identifier.Value;

        public override string ToString() => Name;
    }

    [WhitespaceSurrounded]
    public class Scope : IFunctionBody
    {
        [SequenceTerm, Literal("{")] private Unnamed _open;
        [SequenceTerm, Optional] private List<Item> _items;
        [SequenceTerm, Literal("}")] private Unnamed _close;
    }

    [WhitespaceSurrounded]
    public class Binding : IFunctionBody
    {
        [SequenceTerm, Literal("=")] private Unnamed _bind;
        [SequenceTerm] private Expression _expression;
        [SequenceTerm] private Terminal _terminal;
    }



    public class Function : Item, ICallable, IValue
    {
        [SequenceTerm, Literal("intrinsic"), Optional] private Unnamed _;
        [SequenceTerm] private Declaration _declaration;
        [SequenceTerm] private IFunctionBody _functionBody;

        public override string Name => _declaration.Name;
    }

    [WhitespaceSurrounded]
    public abstract class Item
    {
        public abstract string? Name { get; }
    }
}