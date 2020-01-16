namespace Element
{
	using System;
	using System.Linq;
	using System.Collections.Generic;

	/// <summary>
	/// The global scope, root of all other scopes
	/// </summary>
	public class GlobalScope : IScope
	{
		private readonly List<INamedFunction> _functions;
		private readonly List<INamedType> _types;

		public override string ToString() => "<global>";

		public GlobalScope()
		{
			_types = new List<INamedType>
			{
				NumberType.Instance,
				AnyType.Instance
			};
			_functions = new List<INamedFunction>
			{
				new ArrayIntrinsic(),
				new FoldIntrinsic(),
				new MemberwiseIntrinsic(),
				new ForIntrinsic(),
				new PersistIntrinsic()
			};
			_functions.AddRange(Enum.GetValues(typeof(Binary.Op))
			                        .Cast<Binary.Op>()
			                        .Select(o => new BinaryIntrinsic(o)));
			_functions.AddRange(Enum.GetValues(typeof(Unary.Op)).Cast<Unary.Op>().Select(o => new UnaryIntrinsic(o)));
		}

		/// <summary>
		/// Adds a function to the global scope
		/// </summary>
		public void AddFunction(INamedFunction function, CompilationContext context)
		{
			if (GetFunction(function.Name, context) != null)
			{
				context.LogError(0002, $"Duplicate function named {function.Name}");
			}

			_functions.Add(function);
		}

		/// <summary>
		/// Adds a type to the global scope
		/// </summary>
		public void AddType(INamedType type, CompilationContext context)
		{
			if (FindType(type.Name, context) != null)
			{
				context.LogError(0002, $"Duplicate type named {type.Name}");
			}

			_types.Add(type);
		}

		/// <summary>
		/// Retrieve a function from the context
		/// </summary>
		public INamedFunction GetFunction(string name, CompilationContext context)
		{
			var stack = new CompilationStack();
			if (name.Contains('.'))
			{
				var tokens = name.Split('.');
				IFunction f = GetFunction(tokens[0], context);
				for (var i = 1; i < tokens.Length; i++)
				{
					// Compiles tokens in order of tokens in order to resolve namespaces
					f = ((IScope)f).CompileFunction(tokens[i], stack, context);
				}

				return (INamedFunction)f;
			}
			
			var retval = _functions.Find(b => b.Name == name);
			if (retval == null)
			{
				var type = FindType(name, context);
				if (type != null)
				{
					_functions.Add(retval = new Constructor(type, context));
				}
			}

			return retval;
		}

		public INamedType FindType(string name, CompilationContext context) => _types.Find(b => b.Name == name);

		/// <summary>
		/// Compile a function using everything available in the global scope
		/// </summary>
		public IFunction CompileFunction(string name, CompilationStack stack, CompilationContext context) => GetFunction(name, context);

		/// <summary>
		/// Gets all functions in global scope and any namespaces which match the given filter.
		/// </summary>
		public (string Path, IFunction Function)[] GetAllFunctions(Predicate<IFunction> filter, CompilationContext context)
		{
			IEnumerable<(string, IFunction)> Recurse(string path, IFunction func)
			{ 
				if (func.IsNamespace())
				{
					return func.Outputs.SelectMany(o => Recurse($"{path}.{o.Name}", func.Call(o.Name, context)));
				}

				return filter?.Invoke(func) == false ? Array.Empty<(string, IFunction)>() : new[] {(path, func)};
			}

			return _functions.ToArray().SelectMany(f => Recurse(f.Name, f)).ToArray();
		}
	}
}