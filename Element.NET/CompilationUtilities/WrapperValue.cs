using System;
using System.Collections.Generic;
using Element.AST;
using ResultNET;

namespace Element
{
    public abstract class WrapperValue : IValue
    {
        public override string ToString() => WrappedValue.ToString();
        public IValue WrappedValue { get; }
        protected WrapperValue(IValue result) => WrappedValue = result;
        public virtual string TypeOf => WrappedValue.TypeOf;
        public virtual string SummaryString => WrappedValue.SummaryString;
        public virtual Result<IValue> Call(IReadOnlyList<IValue> arguments, Context context) => WrappedValue.Call(arguments, context);
        public virtual IReadOnlyList<ResolvedPort> InputPorts => WrappedValue.InputPorts;
        public virtual IValue ReturnConstraint => WrappedValue.ReturnConstraint;
        public virtual Result<IValue> Index(Identifier id, Context context) => WrappedValue.Index(id, context);
        public virtual IReadOnlyList<Identifier> Members => WrappedValue.Members;
        public virtual Result MatchesConstraint(IValue value, Context context) => WrappedValue.MatchesConstraint(value, context);
        public virtual Result<IValue> DefaultValue(Context context) => WrappedValue.DefaultValue(context);
        public virtual void Serialize(ResultBuilder<List<Instruction>> resultBuilder, Context context) => WrappedValue.Serialize(resultBuilder, context);
        public virtual Result<IValue> Deserialize(Func<IIntrinsicStructImplementation, Instruction> nextValue, Context context) => WrappedValue.Deserialize(nextValue, context);
        public Result<IValue> InstanceType(Context context) => WrappedValue.InstanceType(context);
        public virtual bool IsIntrinsicOfType<TIntrinsicImplementation>() where TIntrinsicImplementation : IIntrinsicImplementation => WrappedValue.IsIntrinsicOfType<TIntrinsicImplementation>();
        public virtual bool IsSpecificIntrinsic(IIntrinsicImplementation intrinsic) => WrappedValue.IsSpecificIntrinsic(intrinsic);
    }
}