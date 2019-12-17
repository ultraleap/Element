namespace Element.Unity.NodeEditor
{
	using Unity;

	[CreateNodeMenu("Element/Call")]
	public class ElementCallNode : ElementCallNodeBase
	{
		[FunctionName("any", FunctionClass.Function)] public string FunctionName;
		public int VaradicCount;
		[Output] public ElementNodeValue Return;

		protected override string GetFunctionName() => FunctionName;
		protected override int GetVaradicCount() => VaradicCount;
	}
}