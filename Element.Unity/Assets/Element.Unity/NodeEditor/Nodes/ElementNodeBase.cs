namespace Element.Unity.NodeEditor
{
	using XNode;
	using System;

	public abstract class ElementNodeBase : Node
	{
		public abstract string Evaluate(NodePort port, Func<NodePort, string> others);
	}
}