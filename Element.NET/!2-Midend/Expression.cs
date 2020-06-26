using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;

namespace Element
{
	/// <summary>
	/// Base class for all Element expressions
	/// </summary>
	public abstract class Expression : IEquatable<Expression>, ISerializableValue, IIndexable
	{
		protected Expression(IntrinsicType? instanceTypeOverride = default) => Type = instanceTypeOverride ?? NumType.Instance;
		
		/// <summary>
		/// The primitive type of the expression
		/// </summary>
		public readonly IntrinsicType Type;

		public abstract IEnumerable<Expression> Dependent { get; }

		public static string ListJoin(IEnumerable<Expression> list) => string.Join(", ", list.Select(e => e.ToString()));

		public static string StateListJoin(IEnumerable<State> list) => string.Join(", ", list.Select(e => e.InitialValue.ToString()));

		private string? _cachedString;
		protected abstract string ToStringInternal();

		public sealed override string ToString() => _cachedString ??= ToStringInternal();
		public override bool Equals(object obj) => obj is Expression expression && Equals(expression);
		public virtual bool Equals(Expression other) => other?.ToString() == ToString();
		public override int GetHashCode() => ToString().GetHashCode();
		public IEnumerable<Expression> AllDependent => Dependent.SelectMany(d => new[] {d}.Concat(d.AllDependent));
		public int CountUses(Expression other) => Equals(other) ? 1 : Dependent.Sum(d => d.CountUses(other));
		
		public Result<IValue> this[Identifier id, bool recurse, CompilationContext context] =>
			Type.Declaration(context.SourceContext).Bind(decl => decl.ResolveInstanceFunction(id, this, context));

		public void Serialize(ResultBuilder<List<Expression>> resultBuilder) => resultBuilder.Result.Add(this);

		public Result<ISerializableValue> Deserialize(Func<Expression> nextValue, ITrace trace)
		{
			var result = nextValue();
			return result.Type == Type
				       ? new Result<ISerializableValue>(result)
				       : trace.Trace(MessageCode.SerializationError, $"'{result}' deserialized to incorrect type: is '{result.Type}' - expected '{Type}'");
		}
	}
}