using Element.AST;
using System;
using System.Collections.Generic;
using System.IO;
using Newtonsoft.Json.Linq;

namespace Element.JsonArgument
{
	/// <summary>
	/// Provides function arguments from a JObject loaded from file or a json string.
	/// </summary>
	public static class JsonArgumentProvider
	{
		public static Result ProvideArguments(this JObject configuration, IReadOnlyList<ResolvedPort> topLevelFunctionPorts, float[] arguments, Context context)
		{
			var builder = new ResultBuilder(context);
			var idx = 0;

			void ProvisionObject(IReadOnlyList<ResolvedPort> ports, JObject jObject)
			{
				foreach (var input in ports)
				{
					if (input.Identifier.HasValue)
					{
						if (jObject.TryGetValue(input.Identifier.Value.String, out var jToken))
						{
							ProvisionSingleField(input.Identifier.Value, input.ResolvedConstraint, jToken);
						}
					}
					else
					{
						builder.Append(EleMessageCode.InvalidBoundaryFunction, "Port(s) with no identifier(s) which cannot be sourced. Boundary only supports named ports!");
					}
				}
			}
			
			void ProvisionSingleField(Identifier fieldIdentifier, IValue type, JToken value)
			{
				if (type.IsIntrinsic<NumStruct>())
				{
					switch (value?.Type ?? JTokenType.None)
					{
						case JTokenType.Float:
							arguments[idx++] = (float) (double) value!;
							break;
						case JTokenType.Integer:
							arguments[idx++] = (int) value!;
							break;
						default:
							builder.Append(EleMessageCode.InvalidBoundaryData, $"Expected float or integer token for element Num parameter '{fieldIdentifier}'");
							break;
					}
				}
				else if (type.IsIntrinsic<BoolStruct>())
				{
					switch (value?.Type ?? JTokenType.None)
					{
						case JTokenType.Boolean:
							arguments[idx++] = (bool) value! ? 1f : 0f;
							break;
						default:
							builder.Append(EleMessageCode.InvalidBoundaryData, $"Expected boolean token for element Bool parameter '{fieldIdentifier}'");
							break;
					}
				}
				else if (type is Struct declaredStruct)
				{
					switch (value?.Type ?? JTokenType.None)
					{
						case JTokenType.Object:
							ProvisionObject(declaredStruct.Fields, (JObject)value!);
							break;
						default:
							builder.Append(EleMessageCode.InvalidBoundaryData, $"Expected object token for element Struct parameter '{fieldIdentifier}'");
							break;
					}
				}
				else
				{
					builder.Append(EleMessageCode.InvalidBoundaryFunction, $"Element constraint '{type}' is not supported for JSON argument provisioning");
				}
			}

			ProvisionObject(topLevelFunctionPorts, configuration);
			return builder.ToResult();
		}

		public static Result<JObject> ParseFromJsonFile(this string filePath, Context context)
		{
			if (!File.Exists(filePath)) return context.Trace(EleMessageCode.FileAccessError, $"\"{filePath}\" JSON file not found.");
			string fileText;
			try
			{
				fileText = File.ReadAllText(filePath);
			}
			catch (Exception e)
			{
				return context.Trace(EleMessageCode.FileAccessError, e.ToString());
			}
			return ParseFromJsonString(fileText, context);
		}

		public static Result<JObject> ParseFromJsonString(this string json, Context context)
		{
			try
			{
				return JObject.Parse(json);
			}
			catch (Exception e)
			{
				return context.Trace(EleMessageCode.ParseError, e.ToString());
			}
		}
	}
}