using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IStructBody {}
    
    // ReSharper disable once UnusedType.Global
    public class Struct : Item, ICallable, IConstraint, IScope
    {
        [Literal("intrinsic"), Optional] private string _intrinsic;
        [Literal("struct"), WhitespaceSurrounded] private Unnamed _;
        [Term] private Declaration _declaration;
        [Term] private IStructBody _structBody;

        public override Identifier Identifier => _declaration.Identifier;
        protected override DeclaredScope Child => _structBody as Scope;
        public Port[] Inputs => IsAlias ? AliasedInputs : _declaration.PortList?.List.ToArray() ?? Array.Empty<Port>();
        public bool CanBeCached => true;
        public bool IsIntrinsic => !string.IsNullOrEmpty(_intrinsic);
        // ReSharper disable once ConditionIsAlwaysTrueOrFalse
        public bool IsAlias => _declaration.Type != null;
        public override string ToString() => _declaration.ToString();
        private Port[] AliasedInputs { get; set; }

        public bool? MatchesConstraint(IValue value, Port port, CompilationContext compilationContext)
        {
            throw new NotImplementedException();
        }

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = true;
            if (IsAlias)
            {
                // ReSharper disable once ConditionIsAlwaysTrueOrFalse
                if (_declaration.PortList != null)
                {
                    compilationContext.LogError(19, $"Struct alias '{Identifier}' cannot have ports - remove either the ports or the alias type");
                    success = false;
                }

                var aliasee = _declaration.Type.FindConstraint(Parent, compilationContext);
                if (aliasee is Struct constraint)
                {
                    AliasedInputs = constraint.Inputs;
                }
                else
                {
                    compilationContext.LogError(20, $"Cannot create alias of non-struct '{aliasee}'");
                    // ReSharper disable once RedundantAssignment
                    success = false;
                }
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
                success &= scope.ValidateScope(compilationContext);
            }

            return success;
        }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            new StructInstance(Inputs.Select((port, index) => (port.Identifier, arguments[index])));

        public IValue? this[Identifier id] => _structBody is Scope scope ? scope[id] : null;

        IScope? IScope.Parent => Parent;
    }

    public sealed class StructInstance : TransientScope
    {
        public StructInstance(IEnumerable<(Identifier Identifier, IValue Value)> members) => AddRangeToCache(members);
    }
}