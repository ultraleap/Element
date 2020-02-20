using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public interface IStructBody {}

    public interface ITypeIdentity
    {
        // TODO: Remove setter - find another way to handle setting type identity when aliasing primitive types
        IConstraint Identity { get; set; }
    }
    
    // ReSharper disable once UnusedType.Global
    public class Struct : CallableDeclaration<IStructBody>, ICallable, IConstraint, IScope
    {
        protected override string Qualifier { get; } = "struct";
        public bool CanBeCached => true;
        private bool IsAlias => Declaration.Type != null;
        private Struct _aliasedType { get; set; }

        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value switch
        {
            ITypeIdentity i when i.Identity == this => true,
            _ => false
        };

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

                var foundConstraint = FindConstraint(Declaration.Type, compilationContext);
                if (foundConstraint is Struct aliased)
                {
                    _aliasedType = aliased;
                }
                else
                {
                    compilationContext.LogError(20, $"Cannot create alias of non-struct '{foundConstraint}'");
                    success = false;
                }
            }
            else
            {
                success &= ValidateIntrinsic(compilationContext);
                
                if (!IsIntrinsic && DeclaredInputs.Length < 1)
                {
                    compilationContext.LogError(13, $"Non intrinsic '{FullPath}' must have ports");
                    success = false;
                }
            }

            success &= ValidateScopeBody(compilationContext);


            return success;
        }

        private sealed class StructInstance : TransientScope, ITypeIdentity
        {
            public IConstraint Identity { get; set; }

            public StructInstance(Struct identity, Port[] inputs, IEnumerable<IValue> memberValues)
            {
                Identity = identity;
                AddRangeToCache(inputs.Zip(memberValues, (port, value) => (port.Identifier, value)));
            }
        }


        public IValue Call(IValue[] arguments, CompilationContext compilationContext) => Call(arguments, this, compilationContext);

        private IValue Call(IValue[] arguments, Struct instanceType, CompilationContext compilationContext)
        {
            var result = (IsAlias, IsIntrinsic) switch
            {
                (true, _) => _aliasedType.Call(arguments, instanceType, compilationContext),
                (_, true) => ImplementingIntrinsic switch
                {
                    ICallable constructor => constructor.Call(arguments, compilationContext),
                    {} => compilationContext.LogError(14, $"Found intrinsic '{FullPath}' but it is not callable"),
                    _ => compilationContext.LogError(4, $"Intrinsic '{FullPath}' is not implemented")
                },
                (_, _) => new StructInstance(instanceType, DeclaredInputs, arguments)
            };
            (result as ITypeIdentity).Identity = instanceType;
            return result;
        }

        public IValue? this[Identifier id] => Body is Scope scope ? scope[id] : null;

        IScope? IScope.Parent => Parent;
    }
}