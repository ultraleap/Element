namespace Element.Laboratory
{
	using NUnit.Framework;
	
	internal class ArrayIntrinsics : Laboratory.CompilerFixture
	{
		public ArrayIntrinsics(Laboratory.ICommandInterface commandInterface)
			: base(commandInterface) { }

		[Test]
		public void ZeroLengthArray()
		{
			Context.MessageHandler = (messages, anyErrors) =>
			{
				Assert.True(anyErrors);
				messages.Check("ELE0000");
				Assert.Pass();
			};
			Execute("array"); // No arguments
			
			Assert.Fail("Expected message code ELE0000 but execution succeeded");
		}
	}
}