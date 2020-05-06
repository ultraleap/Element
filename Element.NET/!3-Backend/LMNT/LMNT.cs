/*using Element.AST;

namespace Element
{
	using System;
	using System.IO;
	using System.Text;
	using System.Linq;
	using System.Collections.Generic;

	public static class LMNT
	{
		public static void Compile(INamedFunction block, Stream stream, CompilationContext compilationContext) => LoadBlock(block, stream, compilationContext);

		private static uint Pad(int rawSize, int align, out int padding)
		{
			checked
			{
				padding = (align - (rawSize % align)) % align;
				return (uint)(rawSize + padding);
			}
		}

		private class SingleInput : Expression
		{
			public SingleInput(int slot) => Slot = slot;
			public int Slot { get; }
			public override IEnumerable<Expression> Dependent => Array.Empty<Expression>();

			// public override bool Equals(Expression other) => (other as SingleInput)?.Slot == Slot;

			protected override string ToStringInternal() => $"Input{Slot}";
		}

		private static Instruction Compile(Expression expression, int dst, Func<Expression, int> getSrc)
		{
			switch (expression)
			{
				case Binary b:
					return new Instruction
					{
						Opcode = _binary[b.Operation],
						OpA = getSrc(b.OpA),
						OpB = getSrc(b.OpB),
						Dst = dst
					};
				case Unary u:
					return new Instruction
					{
						Opcode = _unary[u.Operation],
						OpA = getSrc(u.Operand),
						Dst = dst
					};
				// TODO: For, persist, mux
				default: throw new NotSupportedException();
			}
		}

		private static void LoadBlock(INamedFunction function, Stream stream, CompilationContext context)
		{
			var inputs = function.Inputs;
			var outputs = function.Outputs;

			var inputSize = 0;
			var arguments = inputs.Select(i => i.Type.Deserialize(() => new SingleInput(inputSize++), context)).ToArray();

			var cache = new Dictionary<Expression, CachedExpression>();
			// Flatten all the outputs into one big array
			var outputExpressions = outputs.SelectMany(o =>
			                               {
				                               var expressions = function.Call(arguments, o.Name, context).Serialize(context);
				                               ConstantFolding.Optimize(expressions);
				                               CommonSubexpressionExtraction.Optimize(expressions,
					                               CommonSubexpressionExtraction.Mode.EveryOperation,
					                               cache);
				                               return expressions;
			                               })
			                               .ToArray();
			var instructions = cache.Values.OrderBy(c => c.Id).ToList();

			// Now each value in outputExpressions should point directly to a CachedExpression or a ConstantExpression
			var constants = outputExpressions.SelectMany(e => e.AllDependent.OfType<Constant>()).Distinct().ToArray();
			var outputSize = outputExpressions.Length;
			var stackSize = instructions.Count + inputSize + constants.Length;

			var name = Encoding.UTF8.GetBytes(function.Name);

			const int headerSize = 24;
			int stringsTableLength = sizeof(ushort) + name.Length + 1;
            const int defsTableLength = (sizeof(ushort) * 8) + (sizeof(uint) * 1) + sizeof(byte);
			int codeTableLength = sizeof(uint) + (sizeof(ushort) * 4 * cache.Count);

			Pad(headerSize + stringsTableLength + defsTableLength + codeTableLength, 8, out var padCode);

			using var bw = new BinaryWriter(stream, Encoding.UTF8);
			checked
			{
				bw.Write('L');
				bw.Write('M');
				bw.Write('N');
				bw.Write('T');
				bw.Write((byte)0); // version major
				bw.Write((byte)0); // version minor
				bw.Write((byte)0); // reserved
				bw.Write((byte)0); // reserved
				bw.Write((uint)stringsTableLength);
				bw.Write((uint)defsTableLength);
				bw.Write((uint)(codeTableLength + padCode));
				bw.Write(sizeof(float) * constants.Length);

				// Strings
				bw.Write((ushort)(name.Length + 1));
				bw.Write(name);
				bw.Write('\0');

				// Definitions
				bw.Write((ushort)((sizeof(ushort) * 8) + (sizeof(uint) * 1) + sizeof(byte)));
				bw.Write((ushort)0);
				bw.Write((ushort)0);
				bw.Write((uint)0);
				bw.Write((ushort)stackSize);
				bw.Write((ushort)stackSize);
				bw.Write((ushort)0);
				bw.Write((ushort)inputSize);
				bw.Write((ushort)outputSize);
				bw.Write((byte)0);

				int getStackSlot(Expression e)
				{
					switch (e)
					{
						case SingleInput s: return s.Slot;
						case Constant c: return Array.IndexOf(constants, c);
						case CachedExpression a:
							var outputIdx = Array.IndexOf(outputExpressions, a);
							if (outputIdx >= 0) return outputIdx;
							return instructions.IndexOf(a);
						default: throw new InvalidOperationException();
					}
				}

				// Code
				bw.Write((uint)instructions.Count);
				foreach (var inst in instructions.Select(c => Compile(c.Value, getStackSlot(c), getStackSlot)))
				{
					bw.Write((ushort)inst.Opcode);
					bw.Write((ushort)inst.OpA);
					bw.Write((ushort)inst.OpB);
					bw.Write((ushort)inst.Dst);
				}

				for (int i = 0; i < padCode; i++) { bw.Write((byte)0); }

				// Constants
				foreach (var c in constants)
					bw.Write(c.Value);
			}
		}

		private enum Opcode
		{
			// no-op: null, null, null
			LMNT_OP_NOOP = 0,

			// copy: stack, imm, stack
			LMNT_OP_COPYSIS,

			// copy: stackref, imm, stack
			LMNT_OP_COPYDIS,

			// copy: stackref, imm, stackref
			LMNT_OP_COPYDID,

			// copy: stackref, stack, stackref
			LMNT_OP_COPYDSD,

			// assign variants: stack, null, stack
			LMNT_OP_ASSIGNSS,
			LMNT_OP_ASSIGNVV,
			LMNT_OP_ASSIGNSV,

			// assign variants: immlo, immhi, stack
			LMNT_OP_ASSIGNIIS,
			LMNT_OP_ASSIGNIBS,
			LMNT_OP_ASSIGNIIV,
			LMNT_OP_ASSIGNIBV,

			// add: stack, stack, stack
			LMNT_OP_ADDSS,
			LMNT_OP_ADDVV,

			// sub: stack, stack, stack
			LMNT_OP_SUBSS,
			LMNT_OP_SUBVV,

			// mul: stack, stack, stack
			LMNT_OP_MULSS,
			LMNT_OP_MULVV,

			// div: stack, stack, stack
			LMNT_OP_DIVSS,
			LMNT_OP_DIVVV,

			// mod: stack, stack, stack
			LMNT_OP_MODSS,
			LMNT_OP_MODVV,

			// trig: stack, null, stack
			LMNT_OP_SIN,
			LMNT_OP_COS,
			LMNT_OP_TAN,
			LMNT_OP_ASIN,
			LMNT_OP_ACOS,
			LMNT_OP_ATAN,

			// pow: stack, stack, stack
			LMNT_OP_POWSS,
			LMNT_OP_POWVV,
			LMNT_OP_POWVS,

			// sqrt: stack, null, stack
			LMNT_OP_SQRTS,
			LMNT_OP_SQRTV,

			// abs: stack, null, stack
			LMNT_OP_ABSS,
			LMNT_OP_ABSV,

			// sum: stack, null, stack
			LMNT_OP_SUMV,

			// min/max: stack, stack, stack
			LMNT_OP_MINSS,
			LMNT_OP_MINVV,
			LMNT_OP_MAXSS,
			LMNT_OP_MAXVV,

			// min/max: stack, null, stack
			LMNT_OP_MINVS,
			LMNT_OP_MAXVS,

			// rounding: stack, null, stack
			LMNT_OP_FLOORS,
			LMNT_OP_FLOORV,
			LMNT_OP_ROUNDS,
			LMNT_OP_ROUNDV,
			LMNT_OP_CEILS,
			LMNT_OP_CEILV,

			// indexing: stackref, immediate, stack
			LMNT_OP_INDEXDIS,

			// indexing: stack, stack, stack
			LMNT_OP_INDEXSSS,

			// indexing: stackref, stack, stack
			LMNT_OP_INDEXDSS,

			// indexing: stackref, stack, stackref
			LMNT_OP_INDEXDSD,

			// extern call: deflo, defhi, imm
			LMNT_OP_EXTCALL,
		}

		private struct Instruction
		{
			public Opcode Opcode;
			public int OpA;
			public int OpB;
			public int Dst;
		}

		private static readonly Dictionary<Binary.Op, Opcode> _binary = new Dictionary<Binary.Op, Opcode>
		{
			{Binary.Op.Add, Opcode.LMNT_OP_ADDSS},
			{Binary.Op.Sub, Opcode.LMNT_OP_SUBSS},
			{Binary.Op.Mul, Opcode.LMNT_OP_MULSS},
			{Binary.Op.Div, Opcode.LMNT_OP_DIVSS},
			{Binary.Op.Rem, Opcode.LMNT_OP_MODSS},
			{Binary.Op.Pow, Opcode.LMNT_OP_POWSS}
		};

		private static readonly Dictionary<Unary.Op, Opcode> _unary = new Dictionary<Unary.Op, Opcode>
		{
			{Unary.Op.Sin, Opcode.LMNT_OP_SIN},
			{Unary.Op.ASin, Opcode.LMNT_OP_ASIN}
		};
	}
}*/