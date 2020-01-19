namespace Element
{
	using System.Linq;
	using Eto.Parse;

	/// <summary>
	/// A custom user-defined Type
	/// </summary>
	internal class CustomType : IntrospectionBase, INamedType
	{
		public CustomType(IScope parent, Match ast, CompilationContext context, string source)
			: base(parent, ast, context, source) { }

		public bool? SatisfiedBy(IFunction value, CompilationContext info)
		{
			var inputs = Inputs.Select(p => p.Type).ToArray();
			// TODO: A better way of doing this!
			var outPorts = Outputs;
			
			// We are a function type
			if (inputs.Length > 0)
			{
				var called = this.Call(inputs, info);
				value = value.Call(inputs, info);
				
				// Calling this type returned another type
				if (called is IType type)
					return type.SatisfiedBy(value, info);
				
				// Calling the type returned something that isn't introspectable
				if (called.Outputs == null)
				{
					// TODO: Check this. What about Abort?
					info.LogError(14, $"Output of {value} is not introspectable");
					return false;
				}

				// Turns a structural thing into a bare list
				// Was required in past due to ambiguity between structural thing and multiple returns
				outPorts = called.Outputs.Select(o => new PortInfo {Name = o.Name, Type = (IType)called.Call(o.Name, info)})
				                 .ToArray();
			}

			// Now we check type satisfaction for each output port
			bool? success = true;
			foreach (var o in outPorts)
			{
				var val = value.Call(o.Name, info);
				if (val is Error)
				{
					success = null;
				}
				else
				{
					var thisSuccess = o.Type.SatisfiedBy(val, info);
					if (thisSuccess == false) {
						info.LogError(14, $"Output {o} not satisfied by {val}");
						success = false;
					}
					else {
						success &= thisSuccess;
					}
				}
			}

			return success;
		}

		public override IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
		{
			return this.CheckArguments(arguments, output, context) ?? Outputs.First(p => p.Name == output).Type;
		}
	}
}