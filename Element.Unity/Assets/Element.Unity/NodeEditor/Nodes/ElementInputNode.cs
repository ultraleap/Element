namespace Element.Unity.NodeEditor
{
	using XNode;
	using System;
	using UnityEngine;

	[CreateNodeMenu("Element/Input")]
	public class ElementInputNode : ElementNodeBase
	{
		[HideInInspector] public ElementContext Context;
		public string Name;
		[FunctionName("any", FunctionClass.Type)] public string Type;
		public float[] TestValue;

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
			var port = GetPort("In");
			if (port == null) {
				AddInstanceOutput(type, ConnectionType.Multiple, "In");
			} else {
				port.ValueType = type;
			}
		}

		public override string Evaluate(NodePort port, Func<NodePort, string> others)
			=> Name;
		
		public override object GetValue(NodePort port)
		{
			var ctx = Context.GetContext();
			var info = new CompilerInfo{};
			var iface = ctx.GetType(Type, info);
			int idx = 0;
			Func<Expression> getNextValue = () => idx < TestValue.Length
				? new Constant(TestValue[idx++]) : Constant.Zero;
			return new ElementNodeValue{Value = iface.Deserialize(getNextValue) };
		}
	}
}