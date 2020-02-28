using System.Linq;

namespace Element.AST
{
    // ReSharper disable once ClassNeverInstantiated.Global
    public class DeclaredStruct : DeclaredItem, ICallable, IScope, IType
    {
        protected override string Qualifier { get; } = "struct";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope), typeof(Terminal)};

        public IValue? this[Identifier id, CompilationContext compilationContext] => Child?[id, compilationContext];
        string IType.Name => Identifier;
        public override IType Type => TypeType.Instance;
        
        public IValue ResolveInstanceFunction(Identifier instanceFunctionIdentifier, IValue instanceBeingIndexed, CompilationContext compilationContext) =>
            this[instanceFunctionIdentifier, compilationContext] switch
            {
                DeclaredFunction instanceFunction when instanceFunction.IsNullary() => compilationContext.LogError(22, $"Constant '{instanceFunction.Location}' cannot be accessed by indexing an instance"),
                IFunction instanceFunction when instanceFunction.Inputs[0].Type.Resolve(this, compilationContext) == this => new InstanceFunction(instanceBeingIndexed, instanceFunction),
                DeclaredItem notInstanceFunction => compilationContext.LogError(22, $"'{notInstanceFunction.Location}' is not a function"),
                {} notInstanceFunction => compilationContext.LogError(22, $"'{notInstanceFunction}' found by indexing '{instanceBeingIndexed}' is not a function"),
                _ => compilationContext.LogError(7, $"Couldn't find any member or instance function '{instanceFunctionIdentifier}' for '{instanceBeingIndexed}' of type <{this}>")
            };

        public bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            IsIntrinsic
                ? IntrinsicCache.GetIntrinsic<IConstraint>(Location, compilationContext)
                                .MatchesConstraint(value, compilationContext)
                : value.Type == this;
        
        public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            IsIntrinsic
                ? ImplementingIntrinsic<ICallable>(compilationContext).Call(arguments, compilationContext)
                : arguments.ValidateArguments(DeclaredInputs, Body as IScope ?? Parent, compilationContext)
                    ? (IValue) new StructInstance(this, DeclaredInputs, arguments)
                    : CompilationErr.Instance;

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = true;
            if (DeclaredType != null)
            {
                compilationContext.LogError(19, $"Struct '{ParsedIdentifier}' cannot have declared return type");
                success = false;
            }
            
            // Intrinsic structs implement constraint resolution and a callable constructor
            // They don't implement IScope, scope impl is still handled by DeclaredStruct
            success &= ValidateIntrinsic<IConstraint>(compilationContext);
            success &= ValidateIntrinsic<ICallable>(compilationContext);
            
            if (!IsIntrinsic && DeclaredInputs.Length < 1)
            {
                compilationContext.LogError(13, $"Non intrinsic '{Location}' must have ports");
                success = false;
            }

            success &= ValidateScopeBody(compilationContext);


            return success;
        }

        private sealed class StructInstance : ScopeBase, IValue, ISerializable
        {
            IType IValue.Type => DeclaringStruct;
            private DeclaredStruct DeclaringStruct { get; }

            public override IValue? this[Identifier id, CompilationContext compilationContext] =>
                IndexCache(id)
                ?? DeclaringStruct.ResolveInstanceFunction(id, this, compilationContext);

            public StructInstance(DeclaredStruct declaringStruct, Port[] inputs, IValue[] memberValues)
            {
                DeclaringStruct = declaringStruct;
                _isSerializable = true;
                _members = new ISerializable[memberValues.Length];
                for (var i = 0; i < memberValues.Length; i++)
                {
                    var value = memberValues[i];
                    if (value is ISerializable serializable)
                    {
                        _members[i] = serializable;
                        SerializedSize += serializable.SerializedSize;
                        continue;
                    }

                    _isSerializable = false;
                    break;
                }

                SetRange(inputs.Zip(memberValues, (port, value) => (port.Identifier, value)));
            }

            private readonly bool _isSerializable;
            private readonly ISerializable[] _members;

            public int SerializedSize { get; }
            public bool Serialize(ref float[] array, ref int position)
            {
                if (!_isSerializable) return false;
                foreach (var m in _members)
                {
                    m.Serialize(ref array, ref position);
                }

                return true;
            }
        }

        private class InstanceFunction : IFunction
        {
            public InstanceFunction(IValue value, IFunction function)
            {
                _surrogate = function;
                _argument = value;
                Inputs = _surrogate.Inputs.Skip(1).ToArray();
            }

            private readonly IFunction _surrogate;
            private readonly IValue _argument;

            public Port[] Inputs { get; }
            public Type Output => _surrogate.Output;
            IType IValue.Type => FunctionType.Instance;

            public IValue Call(IValue[] arguments, CompilationContext compilationContext)
            {
                
                var result = _surrogate.Call(arguments.Prepend(_argument).ToArray(), compilationContext);
                return result.ResolveNullaryFunction(compilationContext);
            }
        }
    }
}