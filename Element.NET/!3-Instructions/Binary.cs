using Element.AST;
using ResultNET;

namespace Element
{
	using System;
	using System.Collections.Generic;

	/// <summary>
	/// 2-arity function, i.e. f(a, b) = expr
	/// </summary>
	public class Binary : Instruction
	{
		public enum Op
		{
			// Bool
			And,
			Or,
			Eq,
			NEq,
			Lt,
			LEq,
			Gt,
			GEq,
			
			// Num
			Add,
			Sub,
			Mul,
			Div,
			Rem,
			Pow,
			Min,
			Max,
			Log,
			Atan2,
		}

		private static readonly HashSet<Op> _boolOps = new HashSet<Op>{
			Op.And,
			Op.Or,
			Op.Eq,
			Op.NEq,
			Op.Lt,
			Op.LEq,
			Op.Gt,
			Op.GEq
		};

		public static Constant Evaluate(Op op, float a, float b) =>
			op switch
			{
				Op.And => a * b > 0f ? Constant.True : Constant.False,
				Op.Or => (a + b) - (a * b) > 0f ? Constant.True : Constant.False,
				Op.Eq => Unary.Evaluate(Unary.Op.Not, Evaluate(Op.NEq, a, b).Value),
				Op.NEq => Math.Abs(a - b) > 0f ? Constant.True : Constant.False,
				Op.Lt => b - a > 0f ? Constant.True : Constant.False,
				Op.Gt => a - b > 0f ? Constant.True : Constant.False,
				Op.LEq => Unary.Evaluate(Unary.Op.Not, Evaluate(Op.Gt, a, b).Value),
				Op.GEq => Unary.Evaluate(Unary.Op.Not, Evaluate(Op.Lt, a, b).Value),
				_ => new Constant(op switch
				{
					Op.Add => (a + b),
					Op.Sub => (a - b),
					Op.Mul => (a * b),
					Op.Div => (a / b),
					Op.Rem => (a % b),
					Op.Pow => (float) Math.Pow(a, b),
					Op.Min => Math.Min(a, b),
					Op.Max => Math.Max(a, b),
					Op.Log => Math.Sign(a) < 0f ? float.NaN : (float) Math.Log(a, b),
					Op.Atan2 => (float) Math.Atan2(a, b),
					_ => throw new ArgumentOutOfRangeException(nameof(op), op, null)
				})
			};

		public static Result<IValue> CreateAndOptimize(Op op, IValue operandA, IValue operandB, Context context)
		{
            Constant NaN() => _boolOps.Contains(op) ? Constant.BoolNaN : Constant.NaN;

            if (!operandA.InnerIs(out Instruction opA)) return context.Trace(ElementMessage.InvalidCompileTarget, $"'{operandA}' is not an Instruction - only Instructions can be binary operands");
            if (!operandB.InnerIs(out Instruction opB)) return context.Trace(ElementMessage.InvalidCompileTarget, $"'{operandB}' is not an Instruction - only Instructions can be binary operands");
            
            var cA = (opA as Constant)?.Value;
            var cB = (opB as Constant)?.Value;
            if (float.IsNaN(cA.GetValueOrDefault()) || float.IsNaN(cB.GetValueOrDefault())) return NaN();
            if (cA.HasValue && cB.HasValue) return Evaluate(op, cA.Value, cB.Value);
            switch (op)
            {
            	case Op.Pow:
            		if (cB == 0f || cA == 1f) { return Constant.One; }
            		if (cA == 0f) { return Constant.Zero; }
            		if (cB == 1f) { return opA; }
            		if (cB == 2f) { return new Binary(Op.Mul, opA, opA); }
            		break;
            	case Op.Add:
            		if (cA == 0f) { return opB; }
            		if (cB == 0f) { return opA; }
            		break;
            	case Op.Sub:
            		if (cB == 0f) { return opA; }
            		break;
            	case Op.Mul:
            		if (cA == 0f || cB == 0f) { return Constant.Zero; }
            		if (cA == 1f) { return opB; }
            		if (cB == 1f) { return opA; }
            		break;
            	case Op.Div:
            		// TODO: Divide by zero warning?
            		if (cB == 1f) { return opA; }
            		if (cB.HasValue) { return new Binary(Op.Mul, opA, new Constant(1/cB.Value)); }
            		break;
            	case Op.Rem:
            		// TODO: Divide by zero warning?
            		if (cA == 0f) { return Constant.Zero; }
            		break;
                case Op.Log:
	                if (cA == 1f) { return Constant.Zero; } // Log_n(1) is always 0
	                if (cA.HasValue && Math.Sign(cA.Value) < 0f) { return NaN(); } // Log_n of any negative is undefined
	                break;
            }
            return new Binary(op, opA, opB);
		}

		private Binary(Op operation, Instruction opA, Instruction opB)
			: base(operation switch
			{
				Op.And => BoolStruct.Instance,
				Op.Or => BoolStruct.Instance,
				Op.Eq => BoolStruct.Instance,
				Op.NEq => BoolStruct.Instance,
				Op.Lt => BoolStruct.Instance,
				Op.LEq => BoolStruct.Instance,
				Op.Gt => BoolStruct.Instance,
				Op.GEq => BoolStruct.Instance,
				_ => default
			})
		{
			Operation = operation;
			OpA = opA ?? throw new ArgumentNullException(nameof(opA));
			OpB = opB ?? throw new ArgumentNullException(nameof(opB));
		}

		public Op Operation { get; }
		public Instruction OpA { get; }
		public Instruction OpB { get; }

		public override IEnumerable<Instruction> Dependent => new[] {OpA, OpB};

		public override string SummaryString => $"{Operation}({OpA}, {OpB})";
		public override int GetHashCode() => (int)Operation ^ OpA.GetHashCode() ^ OpB.GetHashCode();

		public override bool Equals(Instruction other) =>
			// ReSharper disable once PossibleUnintendedReferenceComparison
			this == other || other is Binary bOther
			&& bOther.Operation == Operation
			&& bOther.OpA.Equals(OpA)
			&& bOther.OpB.Equals(OpB);
	}
}