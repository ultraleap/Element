/*using Element.AST;

namespace Element
{
	using System;
	using System.Text;
	using System.Linq;
	using System.Collections.Generic;

	public static class CTarget
	{
		private static readonly Dictionary<Binary.Op, char> _ops = new Dictionary<Binary.Op, char>
		{
			{Binary.Op.Add, '+'},
			{Binary.Op.Sub, '-'},
			{Binary.Op.Mul, '*'},
			{Binary.Op.Div, '/'},
			{Binary.Op.Rem, '%'},
			{Binary.Op.Pow, ','}
		};

		private static readonly Dictionary<Unary.Op, string> _unary = new Dictionary<Unary.Op, string>
		{
			{Unary.Op.Ln, "ln"},
			{Unary.Op.Sin, "sin"},
			{Unary.Op.ASin, "asin"},
		};

		private static void Compile(Expression expression, StringBuilder output,
		                            Dictionary<CachedExpression, StringBuilder> cache)
		{
			switch (expression)
			{
				case SingleInput s:
					output.Append(s);
					break;
				case Constant c:
					output.Append(c.Value);
					break;
				case Binary b:
					if (b.Operation == Binary.Op.Pow)
					{
						output.Append("pow");
					}

					output.Append('(');
					Compile(b.OpA, output, cache);
					output.Append(' ').Append(_ops[b.Operation]).Append(' ');
					Compile(b.OpB, output, cache);
					output.Append(')');
					break;
				case Unary u:
					output.Append(_unary[u.Operation]);
					output.Append('(');
					Compile(u.Operand, output, cache);
					output.Append(')');
					break;
				case CachedExpression v:
					if (!cache.TryGetValue(v, out var _))
					{
						var tmp = new StringBuilder();
						Compile(v.Value, tmp, cache);
						cache.Add(v, tmp);
					}

					output.Append("local_").Append(v.Id);
					break;
				case Mux m:
					output.Append("((float[").Append(m.Operands.Count).Append("]){");
					for (int i = 0; i < m.Operands.Count; i++)
					{
						if (i > 0) { output.Append(", "); }

						Compile(m.Operands[i], output, cache);
					}

					output.Append("}[");
					Compile(m.Selector, output, cache); // TODO: Clamp the indexer
					output.Append(']');
					break;
				// TODO: For, Persist
				default:
					throw new System.NotSupportedException();
			}
		}

		private class SingleInput : Expression
		{
			public SingleInput(string name, int? index) => Name = name;
			public int? Index { get; }

			public string Name { get; }

			// public override bool Equals(Expression other) => this == other;
			public override IEnumerable<Expression> Dependent => Array.Empty<Expression>();
			protected override string ToStringInternal() => Index.HasValue ? $"{Name}[{Index}]" : Name;
		}

		private static IFunction PortToArgument(PortInfo port, Context context, out int size)
		{
			// Special case for singles where we don't want an array at all
			if (port.Type == NumberType.Instance)
			{
				size = 1;
				return new SingleInput(port.Name, null);
			}

			var mySize = 0;
			var ret = port.Type.Deserialize(() => new SingleInput($"_{port.Name}[{mySize}]", mySize++), context);
			size = mySize;
			return ret;
		}

		public static string Compile(INamedFunction function, Context context)
		{
			// Header
			var sb = new StringBuilder();
			sb.Append("#include <math.h>\n\n");

			// Function declaration + evaluation
			sb.Append("void ").Append(function.Name).Append('(');
			var argumentsList = new List<IFunction>();
			foreach (var port in function.InputPorts)
			{
				if (argumentsList.Count > 0)
				{
					sb.Append(", ");
				}

				sb.Append("float _").Append(port.Name);
				var arg = PortToArgument(port, context, out var size);
				if (size > 1)
				{
					sb.Append('[').Append(size).Append(']');
				}

				argumentsList.Add(arg);
			}

			var arguments = argumentsList.ToArray();
			var outputs = new List<(string, Expression[])>();
			foreach (var port in function.Outputs)
			{
				if ((argumentsList.Count + outputs.Count) > 0)
				{
					sb.Append(", ");
				}

				var outValue = function.Call(arguments, port.Name, context).Serialize(context);
				sb.Append("float _").Append(port).Append('[').Append(outValue.Length).Append(']');
				outputs.Add((port.Name, outValue));
			}

			sb.Append(")\n{");

			// Function body + compilation
			var cacheList = new Dictionary<Expression, CachedExpression>();
			var cacheResult = new Dictionary<CachedExpression, StringBuilder>();
			var outputStrings = new StringBuilder();
			foreach (var (name, expressions) in outputs)
			{
				ConstantFolding.Optimize(expressions);
				CommonSubexpressionExtraction.Optimize(expressions,
					CommonSubexpressionExtraction.Mode.OnlyMultipleUses, cacheList);
				for (var i = 0; i < expressions.Length; i++)
				{
					outputStrings.Append("\n\t_").Append(name).Append('[').Append(i).Append("] = ");
					Compile(expressions[i], outputStrings, cacheResult);
					outputStrings.Append(';');
				}
			}

			foreach (var item in cacheList.Values.OrderBy(c => c.Id))
			{
				sb.Append("\n\t")
				  .Append("float local_")
				  .Append(item.Id)
				  .Append(" = ")
				  .Append(cacheResult[item])
				  .Append(';');
			}

			sb.Append(outputStrings).Append("\n}\n");
			return sb.ToString();
		}
	}
}*/