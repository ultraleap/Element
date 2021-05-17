using System;
using System.Collections.Generic;
using System.Linq;
using ResultNET;

namespace Element.AST
{
    public abstract class Struct : Value, IScope
    {
        private readonly ResolvedBlock? _associatedBlock;
        private readonly IScope _parent;

        protected Struct(Identifier identifier, IReadOnlyList<ResolvedPort> fields, ResolvedBlock? associatedBlock, IScope parent)
        {
            Identifier = identifier;
            _associatedBlock = associatedBlock;
            _parent = parent;
            InputPorts = fields;
        }

        public Identifier Identifier { get; }
        public override IReadOnlyList<ResolvedPort> InputPorts { get; }
        public override IValue ReturnConstraint => this;

        public override string SummaryString => Identifier.String;

        public abstract override Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context);
        public abstract override Result MatchesConstraint(IValue value, Context context);
        public override Result<IValue> Index(Identifier id, Context context) => _associatedBlock?.Index(id, context)
                                                                                 ?? (Result<IValue>)context.Trace(EleMessageCode.InvalidExpression, $"'{this}' has no associated scope - it cannot be indexed");
        public Result<IValue> Lookup(Identifier id, Context context) => (_associatedBlock ?? _parent).Lookup(id, context);
        public override IReadOnlyList<Identifier> Members => _associatedBlock?.Members ?? Array.Empty<Identifier>();
        public abstract override Result<IValue> DefaultValue(Context context);
        public bool IsInstanceOfStruct(IValue value, Context context) => value.IsInstanceOfType(this, context);
        public Result<IValue> ResolveInstanceFunction(IValue instance, Identifier id, Context context) =>
            Index(id, context)
                .Bind(v => v switch
                {
                    {} when !v.HasInputs() => context.Trace(EleMessageCode.CannotBeUsedAsInstanceFunction, $"'{v}' found by indexing '{instance}' is not a function"),
                    // ReSharper disable once PossibleUnintendedReferenceComparison
                    {} when v.InputPorts[0].ResolvedConstraint.IsInstance(this) => v.PartiallyApply(new[] {instance}, context),
                    {} => context.Trace(EleMessageCode.CannotBeUsedAsInstanceFunction, $"Found function '{v}' <{v.InputPorts[0]}> must be of type <{Identifier}> to be used as an instance function"),
                    null => throw new InternalCompilerException($"Indexing '{instance}' with '{id}' returned null IValue - this should not occur from user input")
                });
    }
    
    public class IntrinsicStruct : Struct, IIntrinsicValue
    {
        IIntrinsicImplementation IIntrinsicValue.Implementation => _implementation;
        private readonly IIntrinsicStructImplementation _implementation;
        public override bool IsIntrinsicOfType<TIntrinsicImplementation>() => _implementation is TIntrinsicImplementation;
        public override bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic) => _implementation == intrinsic;

        public IntrinsicStruct(IIntrinsicStructImplementation implementation, IReadOnlyList<ResolvedPort> fields, ResolvedBlock? associatedBlock, IScope parent)
            : base(implementation.Identifier, fields, associatedBlock, parent) =>
            _implementation = implementation;

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) => _implementation.Construct(this, arguments, context);
        public override Result MatchesConstraint(IValue value, Context context) => _implementation.MatchesConstraint(this, value, context);
        public override Result<IValue> DefaultValue(Context context) => _implementation.DefaultValue(context);
    }

    public class CustomStruct : Struct
    {
        public CustomStruct(Identifier identifier, IReadOnlyList<ResolvedPort> fields, ResolvedBlock? associatedBlock, IScope parent)
            : base(identifier, fields, associatedBlock, parent) { }

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) => StructInstance.Create(this, arguments, context).Cast<IValue>();
        public override Result MatchesConstraint(IValue value, Context context) => IsInstanceOfStruct(value, context) ? Result.Success : context.Trace(EleMessageCode.ConstraintNotSatisfied, $"Expected {this} instance but got {value}");
        public override Result<IValue> DefaultValue(Context context) =>
            InputPorts.Select(field => field.DefaultValue(context))
                      .BindEnumerable(defaults => Call(defaults.ToArray(), context).Cast<IValue>());
    }
    
    public sealed class StructInstance : Value
    {
        public Struct DeclaringStruct { get; }

        private readonly ResolvedBlock _resolvedBlock;

        public static Result<StructInstance> Create(Struct declaringStruct, IReadOnlyList<IValue> fieldValues, Context context) =>
            declaringStruct.VerifyArgumentsAndApplyFunction(fieldValues, () => new StructInstance(declaringStruct, fieldValues), context)
                           .Cast<StructInstance>();
        
        private StructInstance(Struct declaringStruct, IEnumerable<IValue> fieldValues)
        {
            DeclaringStruct = declaringStruct;
            _resolvedBlock = new ResolvedBlock(declaringStruct.InputPorts.Zip(fieldValues, (port, value) => (port.Identifier!.Value, value)).ToArray(), null, () => this);
        }

        public override string TypeOf => DeclaringStruct.Identifier.String;
        public override Result<IValue> InstanceType(Context context) => DeclaringStruct;
        public override Result<IValue> Index(Identifier id, Context context) =>
            Members.Any(identifier => identifier.Equals(id))
                ? _resolvedBlock.Index(id, context)
                : DeclaringStruct.ResolveInstanceFunction(this, id, context);

        public override IReadOnlyList<Identifier> Members => _resolvedBlock.Members;

        public override void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context)
        {
            if (DeclaringStruct.IsIntrinsicOfType<ListStruct>())
            {
                // TODO: List serialization
                resultBuilder.Append(EleMessageCode.SerializationError, "List serialization not supported yet");
                return;
            }
            
            _resolvedBlock.Serialize(resultBuilder, context);
        }

        public override Result<IValue> Deserialize(Func<Instruction> nextValue, Context context) =>
            _resolvedBlock.DeserializeMembers(nextValue, context)
                          .Bind(deserializedFields => DeclaringStruct.Call(deserializedFields.ToArray(), context));
    }
}