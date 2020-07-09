using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;

namespace Element
{
	/// <summary>
	/// Base class for all Element expressions
	/// </summary>
	// TODO: Make a wrapper Value class for expressions to remove Value logic from Expression implementation
	public abstract class Expression : Value, IEquatable<Expression>
	{
		protected Expression(IntrinsicStructImplementation? instanceStructImplementationOverride = default) => StructImplementation = instanceStructImplementationOverride ?? NumStructImplementation.Instance;
		
		/// <summary>
		/// The primitive type of the expression
		/// </summary>
		public readonly IntrinsicStructImplementation StructImplementation;

		public abstract IEnumerable<Expression> Dependent { get; }

		public static string ListJoin(IEnumerable<Expression> list) => string.Join(", ", list.Select(e => e.ToString()));

		public static string StateListJoin(IEnumerable<State> list) => string.Join(", ", list.Select(e => e.InitialValue.ToString()));
		protected abstract override string ToStringInternal();
		public override bool Equals(object obj) => obj is Expression expression && Equals(expression);
		public virtual bool Equals(Expression other) => other?.ToString() == ToString();
		public override int GetHashCode() => ToString().GetHashCode();
		public IEnumerable<Expression> AllDependent => Dependent.SelectMany(d => new[] {d}.Concat(d.AllDependent));
		public int CountUses(Expression other) => Equals(other) ? 1 : Dependent.Sum(d => d.CountUses(other));

		// TODO: This allows indexing constants from an expression, see note about wrapper class above to fix
		public override Result<IValue> Index(Identifier id, CompilationContext context) =>
			context.SourceContext.GlobalScope.Lookup(StructImplementation.Identifier, context).Bind(v => v.Index(id, context));

		public override void Serialize(ResultBuilder<List<Expression>> resultBuilder, CompilationContext context) => resultBuilder.Result.Add(this);

		public override Result<IValue> Deserialize(Func<Expression> nextValue, CompilationContext context)
		{
			var result = nextValue();
			return result.StructImplementation == StructImplementation
				       ? new Result<IValue>(result)
				       : context.Trace(MessageCode.SerializationError, $"'{result}' deserialized to incorrect type: is '{result.StructImplementation}' - expected '{StructImplementation}'");
		}
	}
}