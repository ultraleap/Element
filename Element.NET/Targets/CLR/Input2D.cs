namespace Element.CLR
{
	using System;
	using System.Collections.Generic;
	using System.Linq;
	using System.Linq.Expressions;

	public class Input2D : ICLRBoundaryMap
	{
		public Input2D(Dictionary<string, object> map) => _map = map;
		private readonly Dictionary<string, object> _map;

		public Expression FromOutput(IFunction output, Type outputType, CompileFunction compile, CompilationContext context)
		{
			var obj = Expression.Variable(outputType, "output_tmp");
			var assigns = new List<Expression>();
			if (!obj.Type.IsValueType)
			{
				assigns.Add(Expression.Assign(obj, Expression.New(outputType)));
			}

			foreach (var pair2d in _map)
			{
				var row = output.Call(pair2d.Key, context);
				if (pair2d.Value is Dictionary<string, string> rowMap)
				{
					foreach (var pair in rowMap)
					{
						var memberExpression = Expression.PropertyOrField(obj, pair.Value);
						var result = compile(row.Call(pair.Key, context), memberExpression.Type, context);
						assigns.Add(Expression.Assign(memberExpression, result));
					}
				}
				else
				{
					var memberExpression = Expression.PropertyOrField(obj, pair2d.Value.ToString());
					assigns.Add(Expression.Assign(memberExpression, compile(row, memberExpression.Type, context)));
				}
			}

			assigns.Add(obj);
			return Expression.Block(new[] {obj}, assigns);
		}

		public IFunction ToInput(Expression parameter, ICLRBoundaryMap rootTypeMap, CompilationContext context) =>
			new Input2DFunc(_map.ToDictionary(p => p.Key,
			                                  p => p.Value is Dictionary<string, string> map
				                                       ? Input1D.Create(parameter, map, rootTypeMap)
				                                       : rootTypeMap.ToInput(
					                                       Expression.PropertyOrField(parameter, p.Value.ToString()),
					                                       rootTypeMap,
					                                       context)));

		private class Input2DFunc : IFunction
		{
			public Input2DFunc(Dictionary<string, IFunction> fields) => _fields = fields;
			public PortInfo[] Inputs => Array.Empty<PortInfo>();
			public PortInfo[] Outputs => _fields.Keys.Select(n => new PortInfo{Name = n, Type = AnyType.Instance}).ToArray();
			private readonly Dictionary<string, IFunction> _fields;

			public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
			{
				return this.CheckArguments(arguments, output, context) ?? _fields[output];
			}
		}
	}
}