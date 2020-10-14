using Element.AST;

namespace Element
{
	using System;
	using System.Collections.Generic;

	/// <summary>
	/// Single arity functions, i.e. f(a) = expr
	/// </summary>
	public class Unary : Instruction
	{
		public enum Op
		{
			// Bool
			Not,
			
			// Num
			Sin,
			Cos,
			Tan,
			ASin,
			ACos,
			ATan,
			Ln,
			Abs,
			Ceil,
			Floor
		}

		public static Constant Evaluate(Op op, float a) => op switch
		{
			Op.Not => float.IsNaN(a) ? Constant.BoolNaN : a > 0f ? Constant.False : Constant.True,
			_ => new Constant(op switch
			{
				Op.Sin => (float) Math.Sin(a),
				Op.Cos => (float) Math.Cos(a),
				Op.Tan => (float) Math.Tan(a),
				Op.ASin => (float) Math.Asin(a),
				Op.ACos => (float) Math.Acos(a),
				Op.ATan => (float) Math.Atan(a),
				Op.Ln => (float) Math.Log(a),
				Op.Abs => Math.Abs(a),
				Op.Ceil => (float) Math.Ceiling(a),
				Op.Floor => (float) Math.Floor(a),
				_ => throw new ArgumentOutOfRangeException()
			})
		};

		public static Instruction CreateAndOptimize(Op op, Instruction operand) => operand is Constant c
			                                                                         ? (Instruction) Evaluate(op, c.Value)
			                                                                         : new Unary(op, operand);
		
		private Unary(Op operation, Instruction operand)
			: base(operation switch
			{
				Op.Not => BoolStruct.Instance,
				_ => NumStruct.Instance
			})
		{
			Operation = operation;
			Operand = operand ?? throw new ArgumentNullException(nameof(operand));
		}

		public Op Operation { get; }
		public Instruction Operand { get; }

		public override IEnumerable<Instruction> Dependent => new[] {Operand};
		public override string SummaryString => $"{Operation}({Operand})";
		public override int GetHashCode() => (int)Operation ^ Operand.GetHashCode();
		public override bool Equals(Instruction other) =>
			// ReSharper disable once PossibleUnintendedReferenceComparison
			this == other || other is Unary bOther
			&& bOther.Operation == Operation
			&& bOther.Operand.Equals(Operand);
	}
}