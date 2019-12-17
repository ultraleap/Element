namespace Element.Unity.NodeEditor
{
	using XNode;
	using UnityEngine;
	using System;
	using System.Linq;
	using System.Collections.Generic;
	using Unity;

	public abstract class ElementCallNodeBase : ElementNodeBase
	{
		[HideInInspector] public ElementContext Context;

		protected abstract string GetFunctionName();
		protected abstract int GetVaradicCount();

		protected override void Init()
		{
			OnValidate();
		}

		protected void OnValidate()
		{
			name = GetFunctionName() ?? "<Call>";
			UpdatePorts();
		}

		public void UpdatePorts()
		{
			if (string.IsNullOrEmpty(GetFunctionName())) { return; }
			var ctx = Context.GetContext();
			var function = ctx.GetFunction(GetFunctionName());
			var existingNames = new HashSet<string>(InstancePorts.Select(p => p.fieldName));
			foreach (var input in function?.Inputs ?? Array.Empty<Element.PortInfo>())
			{
				// TODO: Different types for different Element types
				var type = typeof(ElementNodeValue);
				if (existingNames.Remove(input.Name)) {
					GetPort(input.Name).ValueType = type;
				} else {
					AddInstanceInput(type, ConnectionType.Multiple, input.Name);
				}
			}
			if (function != null && function.Inputs == null)
			{
				for (int i = 0; i < GetVaradicCount(); i++) {
					AddInstanceInput(typeof(object), ConnectionType.Multiple, $"${i}");
				}
			}
			foreach (var name in existingNames) {
				RemoveInstancePort(name);
			}
		}

		public override object GetValue(NodePort port)
		{
			var ctx = Context.GetContext();
			var function = ctx.GetFunction(GetFunctionName());
			if (function == null) return null;
			IEnumerable<string> inputs;
			if (function.Inputs == null) {
				inputs = Enumerable.Range(0, GetVaradicCount())
					.Select(i => $"${i}");
			} else {
				inputs = function.Inputs.Select(p => p.Name);
			}
			var arguments = inputs
				.Select(i => (GetInputPort(i).GetInputValue() as ElementNodeValue)?.Value)
				.ToArray();
			if (arguments.Any(a => a == null)) {
				return null;
			}
			var info = new CompilerInfo{};
			return new ElementNodeValue{
				Value = function.Call(arguments, info)
			};
		}

		public override string Evaluate(NodePort port, Func<NodePort, string> others)
		{
			var ctx = Context.GetContext();
			var function = ctx.GetFunction(GetFunctionName());
			IEnumerable<string> inputs;
			if (function.Inputs == null) {
				inputs = Enumerable.Range(0, GetVaradicCount())
					.Select(i => $"${i}");
			} else {
				inputs = function.Inputs.Select(p => p.Name);
			}
			return $"{GetFunctionName()}({string.Join(", ", inputs.Select(p => GetInputPort(p).Connection).Select(others))})";
		}
	}
}