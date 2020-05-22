using Element.AST;
using System;
using System.IO;
using Newtonsoft.Json.Linq;

namespace Element
{
	/// <summary>
	/// Provides function arguments from a JObject loaded from file or a json string.
	/// </summary>
	public class JsonArgumentProvider
	{
		public JObject JsonConfiguration { get; private set; }

		public void ProvideArguments(IFunction function, float[] arguments, CompilationContext context)
		{
			if (function == null) { return; }

			var idx = 0;
			foreach (var input in function.Inputs)
			{
				if (!input.Identifier.HasValue)
				{
					context.LogError(10, "Function has port(s) with no identifier(s) which cannot be sourced. Boundary only supports named ports!");
					return;
				}
				JsonConfiguration.TryGetValue(input.Identifier, out var iValue);
				RecursivelyEvaluate(arguments, input.Identifier, input.ResolveConstraint(context) as IType, iValue, ref idx, context);
			}
		}

		public (bool Success, string Error) ParseFromJsonFile(string filePath) =>
			!File.Exists(filePath)
				? (false, $"\"{filePath}\" JSON file not found.")
				: ParseFromJsonString(File.ReadAllText(filePath));

		public (bool Success, string Error) ParseFromJsonString(string json)
		{
			try
			{
				JsonConfiguration = JObject.Parse(json);
			}
			catch (Exception e)
			{
				return (false, e.ToString());
			}

			return (true, null);
		}

		private static void RecursivelyEvaluate(float[] argumentArray, string name, IType type, JToken value, ref int idx, CompilationContext context)
		{
			if (type == NumType.Instance)
			{
				argumentArray[idx++] = (value?.Type ?? JTokenType.None) switch
				{
					JTokenType.Float => (float) (double) value,
					JTokenType.Integer => (int) value,
					_ => context.LogError(18, $"Expected float or integer token for element Num parameter '{name}'").Return(0)
				};
			}
			else if (type == BoolType.Instance)
			{
				argumentArray[idx++] = (value?.Type ?? JTokenType.None) switch
				{
					JTokenType.Boolean => (bool)value ? 1f : 0f,
					_ => context.LogError(18, $"Expected boolean token for element Bool parameter '{name}'").Return(0)
				};
			}
			else if (type is StructDeclaration declaredStruct)
			{
				JToken cValue = null;
				(value as JObject)?.TryGetValue(name, out cValue);
				foreach (var field in declaredStruct.Fields)
				{
					RecursivelyEvaluate(argumentArray, field.Identifier, field.ResolveConstraint(declaredStruct, context) as IType, cValue, ref idx, context);
				}
			}
			else
			{
				context.LogError(18, $"Element type '{type}' is not supported for JSON argument provisioning");
			}
		}
	}
}