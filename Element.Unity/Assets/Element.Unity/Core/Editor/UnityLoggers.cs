namespace Element.Unity
{
	using UnityEngine;
	using UnityEditor;
	using ILogger = Element.ILogger;

	public static class UnityLoggers
	{
		public static ILogger HelpBox { get; } = new HelpBoxLog();
		private class HelpBoxLog : ILogger
		{
			public void AddMessage(MessageLevel level, string message)
			{
				EditorGUILayout.HelpBox(message, (MessageType)((int)level + 1));
			}
		}

		public static ILogger Console { get; } = new ConsoleLog();
		private class ConsoleLog : ILogger
		{
			public void AddMessage(MessageLevel level, string message)
			{
				switch (level)
				{
					case MessageLevel.Warning:
						Debug.LogWarning(message);
						break;
					case MessageLevel.Error:
						Debug.LogError(message);
						break;
					default:
						Debug.Log(message);
						break;
				}
			}
		}
	}

	
}