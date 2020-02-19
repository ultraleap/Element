using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IStructBody {}
    
    // ReSharper disable once UnusedType.Global
    public class Struct : CallableDeclaration<IStructBody>, ICallable, IConstraint, IScope
    {
        protected override string Qualifier { get; } = "struct";
        protected override bool IsProxied => IsIntrinsic || IsAlias;
        public bool CanBeCached => true;
        public bool IsAlias => Declaration.Type != null;

        public bool? MatchesConstraint(IValue value, Port port, CompilationContext compilationContext)
        {
            throw new NotImplementedException();
        }

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = true;
            if (IsAlias)
            {
                if (IsIntrinsic)
                {
                    compilationContext.LogError(20, "Intrinsic struct cannot be an alias");
                    success = false;
                }

                // ReSharper disable once ConditionIsAlwaysTrueOrFalse
                if (Declaration.PortList != null)
                {
                    compilationContext.LogError(19, $"Struct alias '{Identifier}' cannot have ports - remove either the ports or the alias type");
                    success = false;
                }

                var aliasee = Declaration.Type.FindConstraint(Parent, compilationContext);
                if (aliasee is Struct constraint)
                {
                    ProxiedInputs = constraint.Inputs;
                }
                else
                {
                    compilationContext.LogError(20, $"Cannot create alias of non-struct '{aliasee}'");
                    success = false;
                }
            }
            else
            {
                success &= ValidateIntrinsic(compilationContext);
                
                if (!IsIntrinsic && Inputs.Length < 1)
                {
                    compilationContext.LogError(13, $"Non intrinsic '{FullPath}' must have ports");
                    success = false;
                }
            }

            success &= ValidateBody(compilationContext);


            return success;
        }

        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            new StructInstance(Inputs.Select((port, index) => (port.Identifier, arguments[index])));

        public IValue? this[Identifier id] => Body is Scope scope ? scope[id] : null;

        IScope? IScope.Parent => Parent;
    }

    public sealed class StructInstance : TransientScope
    {
        public StructInstance(IEnumerable<(Identifier Identifier, IValue Value)> members) => AddRangeToCache(members);
    }
}