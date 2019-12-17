namespace Element.Unity.NodeEditor
{
	using XNode;

	[CreateNodeMenu("Element/Output")]
	public class ElementOutputNode : ElementNodeBase
	{
		public string Name;
		[FunctionName("any", FunctionClass.Type)] public string Type;

		protected override void Init()
		{
			UpdatePorts();
		}

		protected void OnValidate()
		{
			UpdatePorts();
		}
		
		public void UpdatePorts()
		{
			var type = typeof(ElementNodeValue); // TODO: Change based on interface
			var port = GetPort("Out");
			if (port == null) {
				AddInstanceInput(type, ConnectionType.Multiple, "Out");
			} else {
				port.ValueType = type;
			}
		}

		public override string Evaluate(NodePort port, System.Func<NodePort, string> others) => Name;
	}
}