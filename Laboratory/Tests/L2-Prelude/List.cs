using NUnit.Framework;

namespace Laboratory.Tests
{
	/// <summary>
	/// Tests for List and associated functions
	/// </summary>
	internal class List : HostFixture
	{
		public List(IHost host) : base(host) { }

		[Test]
		public void ZeroLengthArrayProducesError()
		{
			var context = new HostContext
			{
				MessageHandler = (messages, anyErrors) =>
				{
					Assert.True(anyErrors);
					messages.Check("ELE0000");
					Assert.Pass();
				}
			};
			_host.Execute(context, "array"); // No arguments
			
			Assert.Fail("Expected message code ELE0000 but execution succeeded");
		}
	}
}