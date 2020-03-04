using System.Linq;

namespace Element.AST
{
    public abstract class DeclaredStruct : DeclaredItem, ICallable, IScope, IType
    {
        protected override string Qualifier { get; } = "struct";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope), typeof(Terminal)};

        public IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] => ChildScope?[id, recurse, compilationContext];
        string IType.Name => Identifier;
        public override IType Type => TypeType.Instance;
        public abstract bool MatchesConstraint(IValue value, CompilationContext compilationContext);
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);

        public IValue ResolveInstanceFunction(Identifier instanceFunctionIdentifier, IValue instanceBeingIndexed, CompilationContext compilationContext) =>
            this[instanceFunctionIdentifier, false, compilationContext] switch
            {
                DeclaredFunction instanceFunction when instanceFunction.IsNullary() => compilationContext.LogError(22, $"Constant '{instanceFunction.Location}' cannot be accessed by indexing an instance"),
                IFunction instanceFunction when instanceFunction.Inputs[0].Type.Resolve(this, compilationContext) == this => new InstanceFunction(instanceBeingIndexed, instanceFunction),
                DeclaredItem notInstanceFunction => compilationContext.LogError(22, $"'{notInstanceFunction.Location}' is not a function"),
                {} notInstanceFunction => compilationContext.LogError(22, $"'{notInstanceFunction}' found by indexing '{instanceBeingIndexed}' is not a function"),
                _ => compilationContext.LogError(7, $"Couldn't find any member or instance function '{instanceFunctionIdentifier}' for '{instanceBeingIndexed}' of type <{this}>")
            };

        public IValue CreateInstance(IValue[] members, IType? instanceType = default) =>
            new StructInstance(this, DeclaredInputs, members, instanceType);

        private sealed class StructInstance : ScopeBase, IValue, ISerializable
        {
            public IType Type { get; }
            private DeclaredStruct DeclaringStruct { get; }

            public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
                IndexCache(id)
                ?? DeclaringStruct.ResolveInstanceFunction(id, this, compilationContext);

            public StructInstance(DeclaredStruct declaringStruct, Port[] inputs, IValue[] memberValues, IType? instanceType = default)
            {
                DeclaringStruct = declaringStruct;
                Type = instanceType ?? declaringStruct;
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

            public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
                _surrogate.Call(arguments.Prepend(_argument).ToArray(), compilationContext)
                    .ResolveNullaryFunction(compilationContext);
        }
    }

    // ReSharper disable once ClassNeverInstantiated.Global
    public class ExtrinsicStruct : DeclaredStruct
    {
        protected override string IntrinsicQualifier => string.Empty;

        public override bool Validate(CompilationContext compilationContext)
        {
            var success = true;
            if (DeclaredType != null)
            {
                compilationContext.LogError(19, $"Struct '{ParsedIdentifier}' cannot have declared return type");
                success = false;
            }

            if (DeclaredInputs.Length < 1)
            {
                compilationContext.LogError(13, $"Non intrinsic '{Location}' must have ports");
                success = false;
            }

            success &= ValidateScopeBody(compilationContext);


            return success;
        }

        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) => value.Type == this;

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            arguments.ValidateArguments(DeclaredInputs, ChildScope ?? ParentScope, compilationContext)
                ? CreateInstance(arguments)
                : CompilationErr.Instance;
    }

    // ReSharper disable once UnusedType.Global
    public class IntrinsicStruct : DeclaredStruct
    {
        protected override string IntrinsicQualifier => "intrinsic";

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
            success &= ImplementingIntrinsic<IConstraint>(compilationContext) != null;
            success &= ImplementingIntrinsic<ICallable>(compilationContext) != null;
            success &= ValidateScopeBody(compilationContext);

            return success;
        }

        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) =>
            ImplementingIntrinsic<IConstraint>(compilationContext).MatchesConstraint(value, compilationContext);

        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
            ImplementingIntrinsic<ICallable>(compilationContext).Call(arguments, compilationContext);
    }
}