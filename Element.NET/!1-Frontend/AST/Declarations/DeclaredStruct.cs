using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public abstract class DeclaredStruct : Declaration, IFunction, IScope, IEnumerable<IValue>, IType
    {
        protected override string Qualifier { get; } = "struct";
        protected override System.Type[] BodyAlternatives { get; } = {typeof(Scope), typeof(Terminal)};
        protected override Identifier[] ScopeIdentifierBlacklist => new[]{Identifier};

        public IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] => ChildScope?[id, recurse, compilationContext];
        public IEnumerator<IValue> GetEnumerator() => ChildScope?.GetEnumerator() ?? Enumerable.Empty<IValue>().GetEnumerator();
        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
        string IType.Name => Identifier;
        public abstract ISerializer? Serializer { get; }
        public override IType Type => TypeType.Instance;
        public Port[] Inputs => DeclaredInputs;
        public Port Output => Port.ReturnPort(this);
        public abstract bool MatchesConstraint(IValue value, CompilationContext compilationContext);
        public abstract IValue Call(IValue[] arguments, CompilationContext compilationContext);

        public IValue ResolveInstanceFunction(Identifier instanceFunctionIdentifier, IValue instanceBeingIndexed, CompilationContext compilationContext) =>
            this[instanceFunctionIdentifier, false, compilationContext] switch
            {
                DeclaredFunction instanceFunction when instanceFunction.IsNullary() => (IValue)compilationContext.LogError(22, $"Constant '{instanceFunction.Location}' cannot be accessed by indexing an instance"), IFunction instanceFunction when instanceFunction.Inputs[0].ResolveConstraint(compilationContext) == this => new InstanceFunction(instanceBeingIndexed, instanceFunction),
                IFunction function => compilationContext.LogError(22, $"Found function '{function}' <{function.Inputs[0]}> must be of type <:{Identifier}> to be used as an instance function"),
                Declaration notInstanceFunction => compilationContext.LogError(22, $"'{notInstanceFunction.Location}' is not a function"),
                {} notInstanceFunction => compilationContext.LogError(22, $"'{notInstanceFunction}' found by indexing '{instanceBeingIndexed}' is not a function"),
                _ => compilationContext.LogError(7, $"Couldn't find any member or instance function '{instanceFunctionIdentifier}' for '{instanceBeingIndexed}' of type <{this}>")
            };

        public IValue CreateInstance(IValue[] members, IType? instanceType = default) =>
            new StructInstance(this, DeclaredInputs, members, instanceType);

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
            public Port Output => _surrogate.Output;

            IType IValue.Type => FunctionType.Instance;

            public IValue Call(IValue[] arguments, CompilationContext compilationContext) =>
                _surrogate.Call(arguments.Prepend(_argument).ToArray(), compilationContext);
        }
    }
    
    public sealed class StructInstance : ScopeBase, IValue
    {
        public IType Type { get; }
        public bool IsSerializable { get; }
        public int SerializedSize { get; }
        private DeclaredStruct DeclaringStruct { get; }

        public override IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
            IndexCache(id)
            ?? DeclaringStruct.ResolveInstanceFunction(id, this, compilationContext);

        public StructInstance(DeclaredStruct declaringStruct, Port[] inputs, IValue[] memberValues, IType? instanceType = default)
        {
            DeclaringStruct = declaringStruct;
            Type = instanceType ?? declaringStruct;
            SetRange(memberValues.WithoutDiscardedArguments(inputs));

            IsSerializable = true;
            foreach (var member in this)
            {
                if (!member.TryGetSerializableSize(out var memberSize))
                {
                    IsSerializable = false;
                    SerializedSize = 0;
                    break;
                }

                SerializedSize += memberSize;
            }
        }
    }

    // ReSharper disable once ClassNeverInstantiated.Global
    public class ExtrinsicStruct : DeclaredStruct
    {
        protected override string IntrinsicQualifier => string.Empty;

        internal override bool Validate(SourceContext sourceContext)
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

        public override ISerializer? Serializer => StructSerializerInstance;
        public static ISerializer StructSerializerInstance { get; } = new StructSerializer();

        private class StructSerializer : ISerializer
        {
            public int SerializedSize(IValue value) => value is StructInstance instance ? instance.SerializedSize : 0;

            public bool Serialize(IValue value, ref float[] array, ref int position)
            {
                if (!(value is StructInstance instance) || !instance.IsSerializable) return false;
                var success = true;
                foreach (var member in instance)
                {
                    success &= member.Type.Serializer.Serialize(member, ref array, ref position);
                }

                return success;
            }
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

        internal override bool Validate(SourceContext sourceContext)
        {
            var success = base.Validate(sourceContext);
            if (DeclaredType != null)
            {
                sourceContext.LogError(19, $"Struct '{Identifier}' cannot have declared return type");
                success = false;
            }

            // Intrinsic structs implement constraint resolution and a callable constructor
            // They don't implement IScope, scope impl is still handled by DeclaredStruct
            success &= ImplementingIntrinsic<IType>(sourceContext) != null;
            success &= ImplementingIntrinsic<IFunction>(sourceContext) != null;

            return success;
        }

        public override ISerializer? Serializer => ImplementingIntrinsic<IType>(null).Serializer;
        public override bool MatchesConstraint(IValue value, CompilationContext compilationContext) => ImplementingIntrinsic<IConstraint>(null).MatchesConstraint(value, compilationContext);
        public override IValue Call(IValue[] arguments, CompilationContext compilationContext) => ImplementingIntrinsic<ICallable>(null).Call(arguments, compilationContext);
    }
}