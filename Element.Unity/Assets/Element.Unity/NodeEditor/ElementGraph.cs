namespace Element.Unity.NodeEditor
{
	using XNode;
	using UnityEngine;
	using System;
	using System.Linq;
	using System.Text;
	using System.Collections.Generic;
	using Unity;

	[CreateAssetMenu(fileName = "New Element Graph", menuName = "Element/Graph")]
	public class ElementGraph : NodeGraph, IElementAsset
	{
		[HideInInspector] public ElementContext Context;

		[ContextMenu("Log Element code")]
		public void ToElementDebug()
		{
			(Context ?? ElementContext.Instance).Reload();
			Debug.Log(ToElement());
		}

		public override Node AddNode(Type type)
		{
			// This is a good place to make sure the graph goes into the context
			var graphs = (Context ?? ElementContext.Instance).Assets;
			if (!graphs.Contains(this)) { graphs.Add(this); }
			return base.AddNode(type);
		}

		public string ToElement()
		{
			var sb = new StringBuilder();
			sb.Append(name.Replace(' ', '_')).Append('(');
			var map = new Dictionary<NodePort, string>();
			var addedNodes = new HashSet<ElementNodeBase>();

			int varId = 0;
			string NextVar() => $"a{varId++}";

			bool first = true;
			// TODO: Input and output ordering
			foreach (var input in nodes.OfType<ElementInputNode>()) {
				//Debug.Log($"Adding {input}'s In as {input.Name}");
				addedNodes.Add(input);
				map.Add(input.GetPort("In"), input.Name);
				if (first) { first = false; }
				else {
					sb.Append(", ");
				}
				sb.Append(input.Name).Append(" : ").Append(input.Type);
			}

			sb.Append(") -> ");

			first = true;
			foreach (var output in nodes.OfType<ElementOutputNode>()) {
				if (first) { first = false; }
				else {
					sb.Append(", ");
				}
				sb.Append(output.Name).Append(" : ").Append(output.Type);
			}

			sb.Append("\n{\n");

			string EvaluatePort(NodePort port)
			{
				if (port == null) { throw new Exception("No input to a required port"); }

				var node = (ElementNodeBase)port.node;
				if (addedNodes.Add(node))
				{
					var driver = node.Evaluate(port, EvaluatePort);
					sb.Append("    ");
					var vName = NextVar();
					sb.Append(vName);
					//Debug.Log($"Adding {node}'s {port.fieldName} as {vName}");
					map.Add(port, vName);
					sb.Append(" = ").Append(driver).Append(";\n");
				}

				if (!map.ContainsKey(port)) { throw new Exception($"Couldn't find a point in the graph for {port.fieldName}"); }

				return map[port];
			}

			foreach (var output in nodes.OfType<ElementOutputNode>()) {
				var connectedPort = output.GetPort("Out").Connection;
				if (connectedPort == null) {
					throw new Exception($"Output {output.Name} has no connection");
				}
				var driver = EvaluatePort(connectedPort);
				sb.Append("    ").Append(output.Name).Append(" = ")
					.Append(driver).Append(";\n");
			}

			sb.Append("\n}");

			return sb.ToString();
		}
	}
}