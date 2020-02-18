using System;
using System.Collections.Generic;
using System.Linq;
using Lexico;

namespace Element.AST
{
    public interface IStructBody {}
    
    public class Struct : Item, ICallable
    {
        [Literal("intrinsic"), Optional] private string _intrinsic;
        [Literal("struct"), WhitespaceSurrounded] private Unnamed _;
        [Term] private Declaration _declaration;
        [Term] private IStructBody _structBody;

        public override Identifier Identifier => _declaration.Identifier;
        public Port[] Inputs => _declaration.PortList?.List.ToArray() ?? Array.Empty<Port>();
        public bool CanBeCached => true;
        public bool IsIntrinsic => !string.IsNullOrEmpty(_intrinsic);
        public bool IsAlias => _declaration.ReturnType != null;
        public override string ToString() => _declaration.ToString();

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = true;

            if (IsAlias)
            {
                if (Inputs.Length > 0)
                {
                    compilationContext.LogError(19, $"Struct alias '{Identifier}' cannot have ports - remove either the ports or the alias type");
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

        public IValue Call(CompilationFrame frame, CompilationContext compilationContext) =>
            new StructInstance(Inputs.Select((port, index) => (port.Identifier, this.GetArgumentByIndex(index, frame, compilationContext))));
    }

    public sealed class StructInstance : ScopeBase
    {
        public StructInstance(IEnumerable<(Identifier Identifier, IValue Value)> members) => AddRangeToCache(members);

        protected override IEnumerable<Item> ItemsToCacheOnValidate => Enumerable.Empty<Item>();
    }
}