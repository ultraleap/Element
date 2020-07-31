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
		protected Expression(IIntrinsicStructImplementation? instanceStructImplementationOverride = default) => StructImplementation = instanceStructImplementationOverride ?? NumStruct.Instance;
		
		/// <summary>
		/// The primitive type of the expression
		/// </summary>
		public readonly IIntrinsicStructImplementation StructImplementation;

		public abstract IEnumerable<Expression> Dependent { get; }

		public static string ListJoinToString(IEnumerable<Expression> list) => string.Join(", ", list.Select(e => e.ToString()));
		public static string ListJoinNormalizedForm(IEnumerable<Expression> list) => string.Join(", ", list.Select(e => e.NormalizedFormString));
		public override Identifier? Identifier => null;
		public abstract override string SummaryString { get; }
		public override string TypeOf => StructImplementation.Identifier.String;
		public override string NormalizedFormString => ListJoinNormalizedForm(Dependent);

		public override bool Equals(object obj) => obj is Expression expression && Equals(expression);
		public virtual bool Equals(Expression other) => other?.ToString() == ToString();
		public override int GetHashCode() => ToString().GetHashCode();
		
		public IEnumerable<Expression> AllDependent => Dependent.SelectMany(d => new[] {d}.Concat(d.AllDependent));
		public int CountUses(Expression other) => Equals(other) ? 1 : Dependent.Sum(d => d.CountUses(other));

		// TODO: This allows indexing constants from an expression, see note about wrapper class above to fix
		public override Result<IValue> Index(Identifier id, Context context) =>
			context.RootScope.Lookup(StructImplementation.Identifier, context).Cast<Struct>(context).Bind(v => v.ResolveInstanceFunction(this, id, context));

		public override void Serialize(ResultBuilder<List<Expression>> resultBuilder, Context context) => resultBuilder.Result.Add(this);

		public override Result<IValue> Deserialize(Func<Expression> nextValue, Context context)
		{
			var result = nextValue();
			return result.StructImplementation == StructImplementation
				       ? new Result<IValue>(result)
				       : context.Trace(MessageCode.SerializationError, $"'{result}' deserialized to incorrect type: is '{result.StructImplementation}' - expected '{StructImplementation}'");
		}
	}
}