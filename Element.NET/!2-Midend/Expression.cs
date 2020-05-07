using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;

namespace Element
{
	/// <summary>
	/// Base class for all Element expressions
	/// </summary>
	public abstract class Expression : IEquatable<Expression>, IValue, IIndexable
	{
		protected Expression(IType? instanceTypeOverride = default) => InstanceTypeOverride = instanceTypeOverride;
		
		public virtual IType Type => InstanceTypeOverride ?? NumType.Instance;

		/// <summary>
		/// The source type of the expression for instance indexing purposes
		/// </summary>
		public readonly IType? InstanceTypeOverride;

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
		
		public IValue? this[Identifier id, bool recurse, CompilationContext compilationContext] =>
			compilationContext.GetIntrinsicsDeclaration<DeclaredStruct>((InstanceTypeOverride ?? Type) as IIntrinsic)
			                  ?.ResolveInstanceFunction(id, this, compilationContext);
	}
}