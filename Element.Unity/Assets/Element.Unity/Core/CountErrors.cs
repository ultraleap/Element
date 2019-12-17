namespace Element.Unity
{
	public class CountErrors : ILogger
	{
		public int ErrorCount { get; private set; }

		public void AddMessage(MessageLevel level, string message)
		{
			if (level == MessageLevel.Error) {
				ErrorCount++;
			}
		}
	}
}