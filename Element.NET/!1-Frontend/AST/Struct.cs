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
        public Port[] Inputs => (IsIntrinsic || IsAlias) ? ProxiedInputs : _declaration.PortList?.List.ToArray() ?? Array.Empty<Port>();
        public bool CanBeCached => true;
        public bool IsIntrinsic => !string.IsNullOrEmpty(_intrinsic);
        public bool IsAlias => _declaration.Type != null;
        public override string ToString() => _declaration.ToString();
        private Port[] ProxiedInputs { get; set; }

        public bool? MatchesConstraint(IValue value, Port port, CompilationContext compilationContext)
        {
            throw new NotImplementedException();
        }

        public override bool Validate(CompilationContext compilationContext)
        {
            IValue success = null;
            if (IsAlias)
            {
                if (IsIntrinsic)
                {
                    success = compilationContext.LogError(20, "Intrinsic struct cannot be an alias");
                }

                // ReSharper disable once ConditionIsAlwaysTrueOrFalse
                if (_declaration.PortList != null)
                {
                    success = compilationContext.LogError(19, $"Struct alias '{Identifier}' cannot have ports - remove either the ports or the alias type");
                }

                var aliasee = _declaration.Type.FindConstraint(Parent, compilationContext);
                if (aliasee is Struct constraint)
                {
                    ProxiedInputs = constraint.Inputs;
                }
                else
                {
                    success = compilationContext.LogError(20, $"Cannot create alias of non-struct '{aliasee}'");
                }
            }
            else
            {
                if (IsIntrinsic)
                {
                    success = GetImplementingIntrinsic(compilationContext);
                    switch (success)
                    {
                        case ICallable callable:
                            ProxiedInputs = callable.Inputs;
                            break;
                        case null: break; // Error already logged by GetImplementingIntrinsic
                        default:
                            success = compilationContext.LogError(14, $"Found intrinsic '{FullPath}' but it is not callable");
                            break;
                    }
                }

                if (!IsIntrinsic && Inputs.Length < 1)
                {
                    success = compilationContext.LogError(13, $"Non intrinsic struct '{Identifier}' must have ports");
                }
            }

            if (_structBody is Scope scope)
            {
                success = scope.ValidateScope(compilationContext) ? null : CompilationErr.Instance;
            }

            return success != CompilationErr.Instance;
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