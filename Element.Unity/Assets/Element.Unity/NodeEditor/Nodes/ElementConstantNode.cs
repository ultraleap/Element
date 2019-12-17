namespace Element.Unity.NodeEditor
{
	using XNode;
	using System;

	[CreateNodeMenu("Element/Constant")]
	public class ElementConstantNode : ElementNodeBase
	{
		public float Value;

		[Output] public ElementNodeValue Output;

		public override object GetValue(NodePort port)
		{
			return new ElementNodeValue{Value = new Constant(Value)};
		}

		public override string Evaluate(NodePort port, Func<NodePort, string> others) => Value.ToString();
	}
}