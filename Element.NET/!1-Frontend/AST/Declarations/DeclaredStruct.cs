using System.Linq;

namespace Element.AST
{
    public abstract class DeclaredStruct : Declaration, IFunction, IScope, IType
    {
        protected override string Qualifier { get; } = "struct";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope), typeof(Terminal)};
        protected override Identifier[] ScopeIdentifierBlacklist => new[]{Identifier};

        public IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] => ChildScope?[id, recurse, compilationContext];
        string IType.Name => Identifier;
        public override IType Type => TypeType.Instance;
        public Port[] Inputs => DeclaredInputs;
        public TypeAnnotation Output => new TypeAnnotation(this);
        public abstract bool MatchesConstraint(IValue value, CompilationContext compilationContext);
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);

        public IValue ResolveInstanceFunction(Identifier instanceFunctionIdentifier, IValue instanceBeingIndexed, CompilationContext compilationContext)
        {
            switch (this[instanceFunctionIdentifier, false, compilationContext])
            {
                case DeclaredFunction instanceFunction when instanceFunction.IsNullary():
                    return compilationContext.LogError(22,
                        $"Constant '{instanceFunction.Location}' cannot be accessed by indexing an instance");
                case IFunction instanceFunction
                    when instanceFunction.Inputs[0].ResolveConstraint(this, compilationContext) == this:
                    return new InstanceFunction(instanceBeingIndexed, instanceFunction);
                case IFunction function:
                    return compilationContext.LogError(22,
                        $"Found function '{function}' <{function.Inputs[0]}> must be of type <:{Identifier}> to be used as an instance function");
                case Declaration notInstanceFunction:
                    return compilationContext.LogError(22, $"'{notInstanceFunction.Location}' is not a function");
                case {} notInstanceFunction:
                    return compilationContext.LogError(22,
                        $"'{notInstanceFunction}' found by indexing '{instanceBeingIndexed}' is not a function");
                default:
                    return compilationContext.LogError(7,
                        $"Couldn't find any member or instance function '{instanceFunctionIdentifier}' for '{instanceBeingIndexed}' of type <{this}>");
            }
        }

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

                SetRange(memberValues.WithoutDiscardedArguments(inputs));
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
            public TypeAnnotation? Output => _surrogate.Output;

            IType IValue.Type => FunctionType.Instance;

            public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
                _surrogate.Call(arguments.Prepend(_argument).ToArray(), compilationContext);
        }
    }

    // ReSharper disable once ClassNeverInstantiated.Global
    public class ExtrinsicStruct : DeclaredStruct
    {
        protected override string IntrinsicQualifier => string.Empty;

        public override bool Validate(SourceContext sourceContext)
        {
            var success = base.Validate(sourceContext);

            if (DeclaredType != null)
            {
                sourceContext.LogError(19, $"Struct '{Identifier}' cannot have declared return type");
                success = false;
            }

            if (!HasDeclaredInputs)
            {
                sourceContext.LogError(13, $"Non intrinsic '{Location}' must have ports");
                success = false;
            }

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

        public override bool Validate(SourceContext sourceContext)
        {
            var success = base.Validate(sourceContext);
            if (DeclaredType != null)
            {
                sourceContext.LogError(19, $"Struct '{Identifier}' cannot have declared return type");
                success = false;
            }

            // Intrinsic structs implement constraint resolution and a callable constructor
            // They don't implement IScope, scope impl is still handled by DeclaredStruct
            success &= ImplementingIntrinsic<IConstraint>(sourceContext) != null;
            success &= ImplementingIntrinsic<IFunction>(sourceContext) != null;

            return success;
        }

        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) => ImplementingIntrinsic<IConstraint>(null).MatchesConstraint(value, compilationContext);
        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) => ImplementingIntrinsic<ICallable>(null).Call(arguments, compilationContext);
    }
}