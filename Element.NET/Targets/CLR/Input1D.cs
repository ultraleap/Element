namespace Element.CLR
{
	using System;
	using System.Collections.Generic;
	using System.Linq;
	using System.Linq.Expressions;

	public class Input1D : ICLRBoundaryMap
	{
		public Input1D(Dictionary<string, string> nameToField) => _map = nameToField;

		public static IFunction Create(Expression parameter, Dictionary<string, string> nameToField, ICLRBoundaryMap rootTypeMap) =>
			new Input1DFunc(parameter, nameToField, rootTypeMap);

		public IFunction ToInput(Expression parameter, ICLRBoundaryMap rootTypeMap, CompilationContext context) =>
			new Input1DFunc(parameter, _map, rootTypeMap);

		public Expression FromOutput(IFunction output, Type outputType, CompileFunction compile, CompilationContext context)
		{
			var obj = Expression.Variable(outputType, "output_tmp");
			var assigns = new List<Expression>();
			if (!obj.Type.IsValueType)
			{
				assigns.Add(Expression.Assign(obj, Expression.New(outputType)));
			}

			foreach (var pair in _map)
			{
				var memberExpression = Expression.PropertyOrField(obj, pair.Value);
				var result = compile(output.Call(pair.Key, context), memberExpression.Type, context);
				assigns.Add(Expression.Assign(memberExpression, result));
			}

			assigns.Add(obj);
			return Expression.Block(new[] {obj}, assigns);
		}

		private readonly Dictionary<string, string> _map;

		private class Input1DFunc : IFunction
		{
			public Input1DFunc(Expression parameter, Dictionary<string, string> map, ICLRBoundaryMap rootTypeMap)
			{
				_parameter = parameter;
				_map = map;
				_rootTypeMap = rootTypeMap;
			}

			private readonly Dictionary<string, string> _map;
			private readonly Expression _parameter;
			private readonly ICLRBoundaryMap _rootTypeMap;

			public PortInfo[] Inputs => Array.Empty<PortInfo>();
			public PortInfo[] Outputs => _map.Keys.Select(n => new PortInfo{Name = n, Type = AnyType.Instance}).ToArray();

			public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
			{
				return this.CheckArguments(arguments, output, context)
					?? _rootTypeMap.ToInput(Expression.PropertyOrField(_parameter, _map[output]), _rootTypeMap, context);
			}
		}
	}
}