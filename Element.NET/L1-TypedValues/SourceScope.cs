using System;
using System.Collections.Generic;
using Element.AST;
using Lexico;

namespace Element
{
    /// <summary>
    /// The global scope, root of all other scopes
    /// </summary>
    [WhitespaceSurrounded, EOFAfter]
    public class SourceScope
    {
	    [Optional] private List<Item> _items;
        
        private readonly Dictionary<string, Item> _cache = new Dictionary<string, Item>();

        /*public GlobalScope AddIntrinsics()
        {
	        // TODO: Add intrinsics to the global scope
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
            _functions.AddRange(Enum.GetValues(typeof(Unary.Op)).Cast<Unary.Op>().Select(o => new UnaryIntrinsic(o)));#1#
        }*/
        
        public override string ToString() => "<global>";

        public bool Validate(CompilationContext compilationContext)
        {
	        var success = true;
	        foreach (var item in _items)
	        {
		        if (!compilationContext.ValidateIdentifier(item.Identifier))
		        {
			        success = false;
			        continue;
		        }

		        if (_cache.ContainsKey(item.Identifier))
		        {
			        compilationContext.LogError(2, $"Cannot add duplicate identifier '{item.Identifier}'");
			        success = false;
		        }
		        else
		        {
			        _cache[item.Identifier] = item;
		        }
	        }

	        return success;
        }

        public Item? this[Identifier id, CompilationContext compilationContext]
        {
	        get
	        {
		        if (_cache.TryGetValue(id, out var item)) return item;
		        item = _items.Find(i => string.Equals(i.Identifier, id, StringComparison.Ordinal));
		        if (item != null)
		        {
			        _cache[id] = item; // Don't cache item if it hasn't been found!
			        compilationContext.LogError(7, $"'{id}' not found in {this}");
		        }
		        return item;
	        }
        }




        /*/// <summary>
		/// Retrieve an identifiable IValue (via compiling it if necessary)
		/// </summary>
		public bool TryGetValue(string name, CompilationContext context, out IValue value)
		{
			if (_identifierToItem.TryGetValue(name, out var item)) return true;
			if (_parseMatches.TryGetValue(name, out var match))
			{
				_values.Add(name, value = match.Compile(this, context));
				return true;
			}

			context.LogError(7, $"Couldn't find identifier '{name}'");
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

			return retval;#1#
		}*/

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