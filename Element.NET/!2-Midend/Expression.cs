using Element.AST;
using System;
using System.Linq;
using System.Collections.Generic;

namespace Element
{
	/// <summary>
	/// Base class for all Element expressions
	/// </summary>
	public abstract class Expression : IEquatable<Expression>, IValue, IIndexable, IFunction, IEvaluable
	{
		protected Expression(AST.IType? instanceTypeOverride = default) => InstanceTypeOverride = instanceTypeOverride;
		
		public AST.IType Type => NumType.Instance;

		/// <summary>
		/// The source type of the expression for instance indexing purposes
		/// </summary>
		public readonly AST.IType? InstanceTypeOverride;

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
			compilationContext.GetIntrinsicsDeclaration<DeclaredStruct>((InstanceTypeOverride ?? AllDependent.FirstOrDefault(d => d.InstanceTypeOverride != null)?.InstanceTypeOverride ?? Type) as IIntrinsic)
			                  ?.ResolveInstanceFunction(id, this, compilationContext);
		
		
		
		// TODO: Remove these
		PortInfo[] IFunction.Inputs => Array.Empty<PortInfo>();
		PortInfo[] IFunction.Outputs => Array.Empty<PortInfo>();
		Expression IEvaluable.AsExpression(CompilationContext info) => this;
		IFunction IFunction.CallInternal(IFunction[] arguments, string output, CompilationContext context) => context.LogError(9999, "Can't call a number");
	}
}