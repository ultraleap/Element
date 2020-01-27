using System.Collections.Generic;

namespace Element
{
	/// <summary>
	/// The global scope, root of all other scopes
	/// </summary>
	public class GlobalScope
	{
		/*private readonly List<INamedFunction> _functions;
		private readonly List<INamedType> _types;*/
		private readonly Dictionary<string, IValue> _values;

		public override string ToString() => "<global>";

		public GlobalScope()
		{
			/*_types = new List<INamedType>
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
			_functions.AddRange(Enum.GetValues(typeof(Unary.Op)).Cast<Unary.Op>().Select(o => new UnaryIntrinsic(o)));*/

			_values = new Dictionary<string, IValue>();
		}

		public bool Add(IValue value, CompilationContext compilationContext)
		{
			if (_values.TryGetValue(value.Identifier, out var found))
			{
				compilationContext.LogError(2, $"Cannot add duplicate value '{value.Identifier}'");
				return false;
			}

			_values.Add(value.Identifier, value);
			return true;

			/*switch (value)
			{
				case null:
					throw new ArgumentNullException(nameof(value));
				case CompilationError _:
					return false;
				case INamedFunction function:
				{
					if (GetFunction(function.Name, context) != null)
					{
						context.LogError(2, $"Duplicate function named {function.Name}");
					}
					else
					{
						_functions.Add(function);
					}

					return true;
				}
				case INamedType type:
				{
					if (FindType(type.Name, context) != null)
					{
						context.LogError(2, $"Duplicate type named {type.Name}");
					}
					else
					{
						_types.Add(type);
					}

					return true;
				}
				default:
					throw new ArgumentOutOfRangeException(nameof(value));
			}*/
		}

		/// <summary>
		/// Retrieve a value from the context
		/// </summary>
		public bool TryGetValue(string name, CompilationContext context, out IValue value)
		{
			if (_values.TryGetValue(name, out value)) return true;
			context.LogError(7, $"Couldn't find {name}");
			return false;

			/*var stack = new CompilationStack();
			if (name.Contains('.'))
			{
				var tokens = name.Split('.');
				IFunction f = GetValue(tokens[0], context);
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

			return retval;*/
		}

		/*public INamedType FindType(string name, CompilationContext context) => _types.Find(b => b.Name == name);

		/// <summary>
		/// Compile a function using everything available in the global scope
		/// </summary>
		public IFunction CompileFunction(string name, CompilationStack stack, CompilationContext context) => GetValue(name, context);*/

		/*/// <summary>
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
		}*/
	}
}