using System.Collections.Generic;
using System.Globalization;
using Lexico;

#pragma warning disable 169,649

namespace Element.AST
{
    public interface IExpressionListStart {}
    public interface IIntrinsic {}
    public interface IBody {}
    public interface IFunction {}

    public class Literal : IExpressionListStart
    {
        private float value;
        public float Value => value;

        public override string ToString() => value.ToString(CultureInfo.CurrentCulture);
    }

    public class Identifier : IExpressionListStart
    {
        [Regex(@"[_\p{L}\p{Nl}][\p{L}\p{Nl}\p{Mn}\p{Mc}\p{Nd}\p{Pc}\p{Cf}]*")] private string value;

        public string Value => value;

        public override string ToString() => value;
    }

    [WhitespaceSurrounded]
    struct ListSeparator
    {
        [Literal(",")] private Unnamed __;
    }

    struct Terminal : IBody
    {
        [Literal(";")] private Unnamed __;
    }

    [WhitespaceSurrounded]
    public abstract class ListOf<T>
    {
        [Literal("(")] private Unnamed __open;
        [SeparatedBy(typeof(ListSeparator))] private List<T> _list;
        [Literal(")")] private Unnamed __close;
    }





    [WhitespaceSurrounded]
    public class Expression
    {
        private IExpressionListStart _start;
        [Optional] private List<CallExpression> _list;
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
        private Identifier _portName;

        public string PortName => _portName.Value;

        public override string ToString() => _portName.ToString();
    }

    [WhitespaceSurrounded]
    public class Declaration
    {
        private Identifier _identifier;
        [Optional] private PortList _portList;
    }

    [WhitespaceSurrounded]
    public class Scope : IBody
    {
        [Literal("{")] private Unnamed __open;
        [Optional] private List<Item> _items;
        [Literal("}")] private Unnamed __close;
    }

    [WhitespaceSurrounded]
    public class Binding : IBody
    {
        [Literal("=")] private Unnamed _bind;
        private Expression _expression;
        private Terminal _terminal;
    }



    public class Function : Item, IFunction
    {
        private Declaration _declaration;
        private IBody _body;
    }

    public class IntrinsicFunction : Item, IFunction, IIntrinsic
    {
        [Literal("intrinsic")] private Unnamed __;
        private Function _function;

        public Function Function => _function;

        public override string ToString() => _function.ToString();
    }

    [WhitespaceSurrounded]
    public abstract class Item { }

    [WhitespaceSurrounded]
    public class Grammar
    {
        private List<Item> Items;
    }
}