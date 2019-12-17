namespace Element.Unity
{
	using System;
	using System.Collections.Generic;

	[Serializable]
	public class CompilationResult : ILogger
	{
		[NonSerialized] public List<string> Errors = new List<string>();

		public void AddMessage(MessageLevel level, string message)
		{
			Errors.Add(message);
		}
	}
}