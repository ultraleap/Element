namespace Element.Unity
{
	using UnityEngine;

	public class FunctionNameAttribute : PropertyAttribute
	{
		public string Type { get; }
		public FunctionClass Class { get; }
		public FunctionNameAttribute(string type, FunctionClass @class) {
			Type = type;
			Class = @class;
		}
	}
}