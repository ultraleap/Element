using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    // ReSharper disable once UnusedType.Global
    public class Struct : DeclaredCallable<IntrinsicStruct>, IConstructor, IConstraint, IScope, IValue
    {
        protected override string Qualifier { get; } = "struct";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope), typeof(Terminal)};
        private bool IsAlias => Declaration.Type != null;
        private Struct _aliasedType { get; set; }

        public IScopeItem? this[Identifier id] => Body is Scope scope ? scope[id] : null;

        IScope? IScope.Parent => Parent;

        // Struct identity is either:
        //    The identity provided by the implementing intrinsic - this allows an intrinsic to control the identity of it's instances
        //        For example, literals declared in source need to have the same type identity as those created using the Num constructor
        //    This struct declaration - normal structs compare values identity reference with the struct declarations instance
        public string TypeIdentity => IsIntrinsic ? GetImplementingIntrinsic(null).FullPath : FullPath;

        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value switch
        {
            { } v when v.TypeIdentity == TypeIdentity => true,
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

                var foundConstraint = Declaration.Type.ResolveConstraint(Body as IScope ?? Parent, compilationContext);
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

        private sealed class StructInstance : ScopeBase, IValue
        {
            public string TypeIdentity { get; }

            public StructInstance(string typeIdentity, Port[] inputs, IEnumerable<IValue> memberValues)
            {
                TypeIdentity = typeIdentity;
                AddRange(inputs.Zip(memberValues, (port, value) => (port.Identifier, value)));
            }
        }


        public IValue Call(IValue[] arguments, CompilationContext compilationContext) => Call(arguments, FullPath, compilationContext);

        public IValue Call(IValue[] arguments, string instanceTypeIdentity, CompilationContext compilationContext) =>
            (IsAlias, IsIntrinsic) switch
            {
                (true, _) => _aliasedType.Call(arguments, instanceTypeIdentity, compilationContext),
                (_, true) => GetImplementingIntrinsic(compilationContext)?.Call(arguments, instanceTypeIdentity, compilationContext),
                (_, _) => arguments.ValidateArgumentCount(DeclaredInputs.Length, compilationContext)
                          && arguments.ValidateArgumentConstraints(DeclaredInputs, Body as IScope ?? Parent, compilationContext)
                              ? (IValue)new StructInstance(instanceTypeIdentity, DeclaredInputs, arguments) : CompilationErr.Instance
            };
    }
}