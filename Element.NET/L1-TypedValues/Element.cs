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
        IValue Call(IValue[] arguments, CompilationFrame frame, CompilationContext compilationContext);
        Port[] Inputs { get; }
    }

    public interface IIndexable : IValue
    {
        IValue? this[Identifier id, CompilationContext compilationContext] { get; }
    }

    public class Literal : IExpressionListStart, IValue
    {
        public Literal() {} // Need parameterless constructor for Lexico to construct instance
        public Literal(float value) {Value = value;}

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
        public override int GetHashCode() => Value.GetHashCode();
        public override bool Equals(object obj) => obj?.Equals(Value) ?? (Value == null);
    }

    [WhitespaceSurrounded]
    struct ListSeparator
    {
        [Literal(",")] private Unnamed _;
        public override string ToString() => ", ";
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
        [field: Optional] public List<CallExpression> CallExpressions { get; } = new List<CallExpression>();

        public override string ToString() => $"{LitOrId}{(CallExpressions != null ? string.Concat(CallExpressions) : string.Empty)}";
    }

    public class CallExpression : ListOf<Expression> { } // CallExpression looks like a list due to using brackets

    public class PortList : ListOf<Port> { }

    [WhitespaceSurrounded]
    public class Port
    {
        [field: Term] public Identifier Identifier { get; set; }

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

        private readonly Dictionary<string, Item> _itemCache = new Dictionary<string, Item>();
        private readonly Dictionary<string, IValue> _valueCache = new Dictionary<string, IValue>();
        protected virtual IEnumerable<Item> ItemsToCacheOnValidate => _items;
            
        public bool Validate(CompilationContext compilationContext)
        {
            var success = true;
            foreach (var item in ItemsToCacheOnValidate)
            {
                if (!compilationContext.ValidateIdentifier(item.Identifier))
                {
                    success = false;
                    continue;
                }

                if (_itemCache.ContainsKey(item.Identifier))
                {
                    compilationContext.LogError(2, $"Cannot add duplicate identifier '{item.Identifier}'");
                    success = false;
                }
                else
                {
                    _itemCache[item.Identifier] = item;
                }
            }

            return success;
        }

        public IValue? this[Identifier id, CompilationContext compilationContext]
        {
            get
            {
                if (_valueCache.TryGetValue(id, out var value)) return value;
                value = _itemCache.TryGetValue(id, out var item) switch
                {
                    true => item switch
                    {
                        IValue v => v,
                        _ => throw new InternalCompilerException($"{item} is not an IValue")
                    },
                    false => compilationContext.LogError(7, $"'{id}' not found")
                };
                if (value != null && value.CanBeCached)
                {
                    _valueCache[id] = value;
                }

                return value;
            }
        }

        public bool CanBeCached => true;
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

        public override string ToString() => Identifier;

        public override Identifier Identifier => _declaration.Identifier;
        public Port[] Inputs => _declaration.PortList?.List.ToArray() ?? Array.Empty<Port>();
        public bool IsIntrinsic => _functionBody is Terminal;

        private static Identifier ReturnIdentifier { get; } = new Identifier("return");

        public sealed class FunctionArguments : IIndexable
        {
            public FunctionArguments(IValue[] arguments, Port[] ports)
            {
                for (var i = 0; i < ports?.Length; i++)
                {
                    _argumentsByIdentifier.Add(ports[i].Identifier, arguments[i]);
                }
            }

            private readonly Dictionary<Identifier, IValue> _argumentsByIdentifier = new Dictionary<Identifier, IValue>();

            public bool CanBeCached => true;

            public IValue? this[Identifier id, CompilationContext compilationContext]
            {
                get { return _argumentsByIdentifier.TryGetValue(id, out var arg) ? arg : null; }
            }
        }


        public IValue Call(IValue[] arguments, CompilationFrame frame, CompilationContext compilationContext)
        {
            IValue CompileFunction(Function function, CompilationFrame callSiteFrame) =>
                function._functionBody switch
                {
                    Binding binding => compilationContext.CompileExpression(binding.Expression, callSiteFrame),
                    Scope scope => scope[ReturnIdentifier, compilationContext] switch
                    {
                        Function dependentFunction => CompileFunction(dependentFunction, callSiteFrame),
                        null => compilationContext.LogError(7, $"'{ReturnIdentifier}' not found in function scope"),
                        var nyi => throw new NotImplementedException(nyi.ToString())
                    },
                    _ => CompilationErr.Instance
                };

            return compilationContext.CheckArguments(arguments, Inputs) ? CompileFunction(this, frame) : CompilationErr.Instance;
        }

        public bool CanBeCached => Inputs == null || Inputs.Length == 0;
    }

    [WhitespaceSurrounded]
    public abstract class Item
    {
        public abstract Identifier Identifier { get; }
    }
}