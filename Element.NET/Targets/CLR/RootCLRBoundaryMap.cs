namespace Element.CLR
{
	using System;
	using System.Collections.Generic;
	using System.Drawing;
	using System.Numerics;
	using LinqExpression = System.Linq.Expressions.Expression;
	using Map = System.Collections.Generic.Dictionary<string, string>;

	/// <summary>
	/// Collection of boundary maps. Will attempt to perform boundary translation using types mapped in the collection.
	/// </summary>
	public class RootCLRBoundaryMap : Dictionary<Type, ICLRBoundaryMap>, ICLRBoundaryMap
	{
		/// <summary>
		/// Create a new map with default mappings
		/// </summary>
		public RootCLRBoundaryMap()
			: this(CreateDefault()) { }

		/// <summary>
		/// Create a new map with custom mappings
		/// </summary>
		public RootCLRBoundaryMap(IDictionary<Type, ICLRBoundaryMap> mappings)
			: base(mappings) { }

		public static Dictionary<Type, ICLRBoundaryMap> CreateDefault()
		{
			var vector2 = new Input1D(new Map {{"x", "X"}, {"y", "Y"}});
			var rect = new Input1D(new Map {{"x", "X"}, {"y", "Y"}, {"width", "Width"}, {"height", "Height"}});
			return new Dictionary<Type, ICLRBoundaryMap>
			{
				{typeof(float), SingleInput.Instance},
				{typeof(int), SingleInput.Instance},
				{typeof(double), SingleInput.Instance},
				{typeof(long), SingleInput.Instance},
				{typeof(bool), SingleInput.Instance},
				{typeof(Vector2), vector2},
				{typeof(Point), vector2},
				{typeof(PointF), vector2},
				{typeof(Vector3), new Input1D(new Map {{"x", "X"}, {"y", "Y"}, {"z", "Z"}})},
				{typeof(Vector4), new Input1D(new Map {{"x", "X"}, {"y", "Y"}, {"z", "Z"}, {"w", "W"}})},
				{typeof(Rectangle), rect},
				{typeof(RectangleF), rect},
				{
					typeof(Matrix4x4), new Input2D(new Dictionary<string, object>
					{
						{"x", new Map {{"x", "M11"}, {"y", "M12"}, {"z", "M13"}, {"w", "M14"}}},
						{"y", new Map {{"x", "M21"}, {"y", "M22"}, {"z", "M23"}, {"w", "M24"}}},
						{"z", new Map {{"x", "M31"}, {"y", "M32"}, {"z", "M33"}, {"w", "M34"}}},
						{"w", new Map {{"x", "M41"}, {"y", "M42"}, {"z", "M43"}, {"w", "M44"}}}
					})
				},
				{
					typeof(Matrix3x2), new Input2D(new Dictionary<string, object>
					{
						{"x", new Map {{"x", "M11"}, {"y", "M12"}}},
						{"y", new Map {{"x", "M21"}, {"y", "M22"}}},
						{"z", new Map {{"x", "M31"}, {"y", "M32"}}}
					})
				},
				{typeof(float[]), new Array1D()}
			};
		}

		public IFunction ToInput(LinqExpression parameter, ICLRBoundaryMap rootTypeMap, CompilationContext context)
		{
			if (TryGetValue(parameter.Type, out var retval))
			{
				return retval.ToInput(parameter, rootTypeMap, context);
			}

			return context.LogError($"No {nameof(ICLRBoundaryMap)} found for CLR type {parameter.Type}");
		}

		public LinqExpression FromOutput(IFunction output, Type outputType,
		                                 CompileFunction compile, CompilationContext context)
		{
			// TODO: Detect circular
			if (TryGetValue(outputType, out var retval))
			{
				return retval.FromOutput(output, outputType, compile, context);
			}

			context.LogError($"No {nameof(ICLRBoundaryMap)} found for CLR type {outputType}");
			return null;
		}
	}
}