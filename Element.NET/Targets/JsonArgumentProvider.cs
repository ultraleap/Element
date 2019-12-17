namespace Element
{
	using System;
	using System.IO;
	using Newtonsoft.Json.Linq;

	/// <summary>
	/// Provides function arguments from a JObject loaded from file or a json string.
	/// </summary>
	public class JsonArgumentProvider
	{
		public JObject JsonConfiguration { get; private set; }

		public void ProvideArguments(IFunction function, float[] arguments)
		{
			if (function == null) { return; }

			var idx = 0;
			var info = new CompilationContext();
			foreach (var input in function.Inputs)
			{
				JsonConfiguration.TryGetValue(input.Name, out var iValue);
				RecursivelyEvaluate(arguments, input.Type, iValue, ref idx, info);
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

		private static void RecursivelyEvaluate(float[] config, IFunction func, JToken value, ref int idx, CompilationContext info)
		{
			if (func.IsLeaf())
			{
				switch (value?.Type ?? JTokenType.None)
				{
					case JTokenType.Float:
						config[idx++] = (float)(double)value;
						break;
					case JTokenType.Integer:
						config[idx++] = (int)value;
						break;
					default:
						config[idx++] = 0;
						break;
				}
			}
			else
			{
				foreach (var output in func.Outputs)
				{
					JToken cValue = null;
					(value as JObject)?.TryGetValue(output.Name, out cValue);
					RecursivelyEvaluate(config, func.Call(output.Name, info), cValue, ref idx, info);
				}
			}
		}
	}
}