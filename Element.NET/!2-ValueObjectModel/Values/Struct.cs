using System;
using System.Collections.Generic;
using System.Linq;

namespace Element.AST
{
    public abstract class Struct : Value, IScope, IFunctionValue
    {
        private readonly ResolvedBlock? _associatedBlock;
        private readonly IScope _parent;

        protected Struct(IReadOnlyList<ResolvedPort> fields, ResolvedBlock? associatedBlock, IScope parent, string? location = null) : base(location)
        {
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
    }
    
    public class IntrinsicStruct : Struct, IIntrinsicValue
    {
        IIntrinsicImplementation IIntrinsicValue.Implementation => _implementation;
        private readonly IIntrinsicStructImplementation _implementation;

        public IntrinsicStruct(IIntrinsicStructImplementation implementation, IReadOnlyList<ResolvedPort> fields, ResolvedBlock? associatedBlock, IScope parent, string location)
            : base(fields, associatedBlock, parent, location) =>
            _implementation = implementation;

        public override Result<IValue> Call(IReadOnlyList<IValue> arguments, CompilationContext context) => _implementation.Construct(this, arguments, context);
        public override Result<bool> MatchesConstraint(IValue value, CompilationContext context) => _implementation.MatchesConstraint(this, value, context);
        public override Result<IValue> DefaultValue(CompilationContext context) => _implementation.DefaultValue(context);
    }

    public class CustomStruct : Struct
    {
        public CustomStruct(IReadOnlyList<ResolvedPort> fields, ResolvedBlock? associatedBlock, IScope parent, string location)
            : base(fields, associatedBlock, parent, location) { }
        
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

        protected override string ToStringInternal() => $"{DeclaringStruct}:Instance";
        
        public override Result<IValue> Index(Identifier id, CompilationContext context)
        {
            Result<IValue> ResolveInstanceFunction() =>
                DeclaringStruct.Index(id, context)
                               .Bind(v => v switch
                               {
                                   IFunctionValue function when function.IsNullary() => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"Constant '{function}' cannot be accessed by indexing an instance"),
                                   IFunctionValue function when function.InputPorts[0].ResolvedConstraint == this => function.PartiallyApply(new IValue[] {this}, context),
                                   IFunctionValue function => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"Found function '{function}' <{function.InputPorts[0]}> must be of type <{this}> to be used as an instance function"),
                                   {} notFunction => context.Trace(MessageCode.CannotBeUsedAsInstanceFunction, $"'{notFunction}' found by indexing '{this}' is not a function"),
                                   null => throw new InternalCompilerException($"Indexing '{this}' with '{id}' returned null IValue - this should not occur from user input")
                               });

            return _resolvedBlock.Index(id, context)
                         .Else(ResolveInstanceFunction);
        }

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

        public override Result<IValue> Deserialize(Func<Element.Expression> nextValue, CompilationContext context) => _resolvedBlock.DeserializeMembers(nextValue, context)
                                                                                                                            .Map(deserializedFields => (IValue) new StructInstance(DeclaringStruct, deserializedFields));
    }
}