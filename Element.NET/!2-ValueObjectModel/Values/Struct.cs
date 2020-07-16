using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public abstract class Struct : Value, IScope, IFunctionValue
    {
        public readonly Identifier Identifier;
        private readonly ResolvedBlock? _associatedBlock;
        private readonly IScope _parent;

        protected Struct(Identifier identifier, IReadOnlyList<ResolvedPort> fields, ResolvedBlock? associatedBlock, IScope parent)
        {
            Identifier = identifier;
            _associatedBlock = associatedBlock;
            _parent = parent;
            Fields = fields;
        }
        
        IReadOnlyList<ResolvedPort> IFunctionSignature.InputPorts => Fields;
        IValue IFunctionSignature.ReturnConstraint => this;
        public IReadOnlyList<ResolvedPort> Fields { get; }

        public abstract override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context);
        public abstract override Result<bool> MatchesConstraint(IValue value, CompilationContext context);
        public override Result<IValue> Index(Identifier id, CompilationContext context) => _associatedBlock?.Index(id, context)
                                                                                           ?? (Result<IValue>)context.Trace(MessageCode.InvalidExpression, $"'{this}' has no associated scope - it cannot be indexed");
        public Result<IValue> Lookup(Identifier id, CompilationContext context) => (_associatedBlock ?? _parent).Lookup(id, context);
        public override IReadOnlyList<Identifier> Members => _associatedBlock?.Members ?? Array.Empty<Identifier>();
        public abstract override Result<IValue> DefaultValue(CompilationContext context);
        public Result<bool> IsInstanceOfStruct(IValue value, CompilationContext context) => value.FullyResolveValue(context).Map(v => v is StructInstance instance && instance.DeclaringStruct == this);
        public Result<IValue> ResolveInstanceFunction(IValue instance, Identifier id, CompilationContext context) =>
            Index(id, context)
                .Bind(v => v switch
                {
                    IFunctionValue function when function.IsNullary() => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"Constant '{function}' cannot be accessed by indexing an instance"),
                    IFunctionValue function when function.InputPorts[0].ResolvedConstraint == this => function.PartiallyApply(new[] {instance}, context),
                    IFunctionValue function => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"Found function '{function}' <{function.InputPorts[0]}> must be of type <{Identifier}> to be used as an instance function"),
                    {} notFunction => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"'{notFunction}' found by indexing '{instance}' is not a function"),
                    null => throw new InternalCompilerException($"Indexing '{instance}' with '{id}' returned null IValue - this should not occur from user input")
                });
    }
    
    public class IntrinsicStruct : Struct, IIntrinsicValue
    {
        IIntrinsicImplementation IIntrinsicValue.Implementation => _implementation;
        private readonly IIntrinsicStructImplementation _implementation;

        public IntrinsicStruct(IIntrinsicStructImplementation implementation, IReadOnlyList<ResolvedPort> fields, ResolvedBlock? associatedBlock, IScope parent)
            : base(implementation.Identifier, fields, associatedBlock, parent) =>
            _implementation = implementation;

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) => _implementation.Construct(this, arguments, context);
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => _implementation.MatchesConstraint(this, value, context);
        public override Result<IValue> DefaultValue(CompilationContext context) => _implementation.DefaultValue(context);
    }

    public class CustomStruct : Struct
    {
        public CustomStruct(Identifier identifier, IReadOnlyList<ResolvedPort> fields, ResolvedBlock? associatedBlock, IScope parent)
            : base(identifier, fields, associatedBlock, parent) { }

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) => new StructInstance(this, arguments);
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => IsInstanceOfStruct(value, context);
        public override Result<IValue> DefaultValue(CompilationContext context) =>
            Fields.Select(field => field.DefaultValue(context))
                  .MapEnumerable(defaults => (IValue)new StructInstance(this, defaults.ToArray()));
    }
    
    public sealed class StructInstance : Value
    {
        public Struct DeclaringStruct { get; }

        private readonly ResolvedBlock _resolvedBlock;
        
        public StructInstance(Struct declaringStruct, IEnumerable<IValue> fieldValues)
        {
            DeclaringStruct = declaringStruct;
            _resolvedBlock = new ResolvedBlock(declaringStruct.Fields.Zip(fieldValues, (port, value) => (port.Identifier!.Value, value)).ToArray(), null);
        }

        protected override string ToStringInternal() => $"StructInstance:{DeclaringStruct.Identifier}";

        public override Result<IValue> Index(Identifier id, CompilationContext context) =>
            _resolvedBlock.Index(id, context)
                          .Else(() => DeclaringStruct.ResolveInstanceFunction(this, id, context));

        public override IReadOnlyList<Identifier> Members => _resolvedBlock.Members;

        public override void Serialize(ResultBuilder<List<Element.Expression>> resultBuilder, CompilationContext context)
        {
            if (DeclaringStruct.IsIntrinsic<ListStruct>())
            {
                // TODO: List serialization
                resultBuilder.Append(MessageCode.SerializationError, "List serialization not supported yet");
                return;
            }
            
            _resolvedBlock.Serialize(resultBuilder, context);
        }

        public override Result<IValue> Deserialize(Func<Element.Expression> nextValue, CompilationContext context) =>
            _resolvedBlock.DeserializeMembers(nextValue, context)
                          .Map(deserializedFields => (IValue) new StructInstance(DeclaringStruct, deserializedFields));
    }
}