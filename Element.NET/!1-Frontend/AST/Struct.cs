using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IStructBody {}
    
    public class Struct : Item, ICallable, IConstraint
    {
        [Literal("intrinsic"), Optional] private string _intrinsic;
        [Literal("struct"), WhitespaceSurrounded] private Unnamed _;
        [Term] private Declaration _declaration;
        [Term] private IStructBody _structBody;

        public override Identifier Identifier => _declaration.Identifier;
        public Port[] Inputs => IsAlias ? _aliasedInputs : _declaration.PortList?.List.ToArray() ?? Array.Empty<Port>();
        public bool CanBeCached => true;
        public bool IsIntrinsic => !string.IsNullOrEmpty(_intrinsic);
        public bool IsAlias => _declaration.Type != null;
        public override string ToString() => _declaration.ToString();
        private Port[] _aliasedInputs { get; set; }

        public bool? MatchesConstraint(IValue value, Port port, CompilationContext compilationContext)
        {
            throw new NotImplementedException();
        }

        public override bool Validate(IIndexable parent, CompilationContext compilationContext)
        {
            var success = true;
            if (IsAlias)
            {
                if (_declaration.PortList != null)
                {
                    compilationContext.LogError(19, $"Struct alias '{Identifier}' cannot have ports - remove either the ports or the alias type");
                    success = false;
                }

                /*var aliasee = _declaration.Type.GetConstraint(frame, compilationContext);
                if (aliasee is Struct constraint)
                {
                    inputs = constraint.Inputs;
                }
                else
                {
                    compilationContext.LogError(20, $"Cannot create alias of non-struct '{aliasee}'");
                }*/
            }
            else
            {
                if (!IsIntrinsic && Inputs.Length < 1)
                {
                    compilationContext.LogError(13, $"Non intrinsic struct '{Identifier}' must have ports");
                    success = false;
                }
            }

            if (_structBody is Scope scope)
            {
                success &= scope.ValidateScope(parent, compilationContext);
            }

            return success;
        }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            new StructInstance(Inputs.Select((port, index) => (port.Identifier, arguments[index])));
    }

    public sealed class StructInstance : ScopeBase
    {
        public StructInstance(IEnumerable<(Identifier Identifier, IValue Value)> members) => AddRangeToCache(members);

        protected override IEnumerable<Item> ItemsToCacheOnValidate => Enumerable.Empty<Item>();
    }
}