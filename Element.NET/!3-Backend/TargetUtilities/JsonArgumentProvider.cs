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

		public Result ProvideArguments(IFunctionSignature functionSignature, float[] arguments, CompilationContext context)
		{
			var idx = 0;
			int Index() => idx++; 
			var resultBuilder = new ResultBuilder(context);
			foreach (var input in functionSignature.Inputs)
			{
				if (input.Identifier.HasValue)
				{
					JsonConfiguration.TryGetValue(input.Identifier.Value, out var iValue);
					RecursivelyEvaluate(resultBuilder, arguments, input.Identifier.Value, input.ResolveConstraint(context), iValue, Index, context);
				}
				else
				{
					resultBuilder.Append(MessageCode.InvalidBoundaryFunction, "Function has port(s) with no identifier(s) which cannot be sourced. Boundary only supports named ports!");
				}
			}

			return resultBuilder.ToResult();
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

			return (true, string.Empty);
		}

		private static void RecursivelyEvaluate(ResultBuilder builder, float[] argumentArray, Identifier fieldIdentifier, in Result<IValue> fieldType, JToken? value, Func<int> idx, CompilationContext context) =>
			fieldType.Do(type =>
			{
				if (type.IsIntrinsicType<NumType>())
				{
					switch (value?.Type ?? JTokenType.None)
					{
						case JTokenType.Float:
							argumentArray[idx()] = (float) (double) value;
							break;
						case JTokenType.Integer:
							argumentArray[idx()] = (int) value;
							break;
						default:
							builder.Append(MessageCode.InvalidBoundaryData, $"Expected float or integer token for element Num parameter '{fieldIdentifier}'");
							break;
					}
				}
				else if (type.IsIntrinsicType<BoolType>())
				{
					switch (value?.Type ?? JTokenType.None)
					{
						case JTokenType.Boolean:
							argumentArray[idx()] = (bool) value ? 1f : 0f;
							break;
						default:
							builder.Append(MessageCode.InvalidBoundaryData, $"Expected boolean token for element Bool parameter '{fieldIdentifier}'");
							break;
					}
				}
				else if (type is StructDeclaration declaredStruct)
				{
					JToken? cValue = null;
					(value as JObject)?.TryGetValue(fieldIdentifier, out cValue);
					foreach (var field in declaredStruct.Fields)
					{
						if (field.Identifier.HasValue)
						{
							RecursivelyEvaluate(builder, argumentArray, field.Identifier.Value, field.ResolveConstraint(declaredStruct, context), cValue, idx, context);
						}
						else
						{
							builder.Append(MessageCode.InvalidBoundaryData, $"Struct '{declaredStruct}' has field(s) with no identifier(s) which cannot be sourced. Boundary only supports named ports!");
						}
					}
				}
				else
				{
					builder.Append(MessageCode.InvalidBoundaryData, $"Element type '{type}' is not supported for JSON argument provisioning");
				}
			});
	}
}