using Element.AST;

namespace Element
{
	using System;
	using System.Linq;
	using System.Collections.Generic;

	/// <summary>
	/// Base class for all Element expressions
	/// </summary>
	public abstract class Expression : IEquatable<Expression>, IFunction, IEvaluable, IValue
	{
		PortInfo[] IFunction.Inputs => Array.Empty<PortInfo>();
		PortInfo[] IFunction.Outputs => Array.Empty<PortInfo>();
		Expression IEvaluable.AsExpression(CompilationContext info) => this;

		IFunction IFunction.CallInternal(IFunction[] arguments, string output, CompilationContext context) => context.LogError(9999, "Can't call a number");

		public abstract IEnumerable<Expression> Dependent { get; }

		public static string ListJoin(IEnumerable<Expression> list) => string.Join(", ", list.Select(e => e.ToString()));

		public static string StateListJoin(IEnumerable<State> list) => string.Join(", ", list.Select(e => e.InitialValue.ToString()));

		private string _cachedString;
		protected abstract string ToStringInternal();

		public sealed override string ToString() => _cachedString ??= ToStringInternal();
		public override bool Equals(object obj) => obj is Expression expression && Equals(expression);
		public override int GetHashCode() => ToString().GetHashCode();

		public IEnumerable<Expression> AllDependent => Dependent.SelectMany(d => new[] {d}.Concat(d.AllDependent));
		public int CountUses(Expression other) => Equals(other) ? 1 : Dependent.Sum(d => d.CountUses(other));

		public virtual bool Equals(Expression other) => other?.ToString() == ToString();
		public bool CanBeCached => true;
	}
}