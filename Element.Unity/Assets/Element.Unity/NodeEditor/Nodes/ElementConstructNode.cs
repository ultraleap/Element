namespace Element.Unity.NodeEditor
{
	using Unity;

	[CreateNodeMenu("Element/Construct")]
	public class ElementConstructNode : ElementCallNodeBase
	{
		[FunctionName("any", FunctionClass.Constructor)] public string Type;
		[Output] public ElementNodeValue Return;

		protected override string GetFunctionName() => Type;
		protected override int GetVaradicCount() => 0;
	}
}