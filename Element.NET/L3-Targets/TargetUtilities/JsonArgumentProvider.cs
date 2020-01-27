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

		public void ProvideArguments(IFunction function, float[] arguments, CompilationContext context)
		{
			if (function == null) { return; }

			var idx = 0;
			foreach (var input in function.Inputs)
			{
				JsonConfiguration.TryGetValue(input.Name, out var iValue);
				RecursivelyEvaluate(arguments, input.Type, iValue, ref idx, context);
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

		private static void RecursivelyEvaluate(float[] config, IFunction func, JToken value, ref int idx, CompilationContext context)
		{
			if (func.IsLeaf())
			{
				config[idx++] = (value?.Type ?? JTokenType.None) switch
				{
					JTokenType.Float => (float) (double) value,
					JTokenType.Integer => (int) value,
					_ => 0
				};
			}
			else
			{
				foreach (var output in func.Outputs)
				{
					JToken cValue = null;
					(value as JObject)?.TryGetValue(output.Name, out cValue);
					RecursivelyEvaluate(config, func.Call(output.Name, context), cValue, ref idx, context);
				}
			}
		}
	}
}