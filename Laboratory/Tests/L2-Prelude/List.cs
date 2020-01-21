using Element;
using Element.CLR;
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
			var input = new CompilationInput(false, false, null,
			TestContext.WriteLine,
			err => PassIfMessageCodeFound(err, 13));

			_host.Execute(input, "array"); // No arguments

			Assert.Fail("Expected message code ELE13 but execution succeeded");
		}
	}
}