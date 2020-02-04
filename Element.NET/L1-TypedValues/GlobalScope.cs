using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;
using Lexico;

namespace Element
{
    /// <summary>
    /// The global scope, root of all other scopes
    /// </summary>
    [WhitespaceSurrounded, EOFAfter]
    public class GlobalScope : ICompilationScope
    {
        [SequenceTerm, Optional] private List<Item> _items;
        
        private readonly Dictionary<string, Item> _identifierToItem = new Dictionary<string, Item>();
        
        public override string ToString() => "<global>";
        
        public IValue Compile(string identifier, CompilationContext context)
        {
	        if (string.IsNullOrEmpty(identifier))
	        {
		        context.LogError(15, "Cannot compile a null or empty identifier");
		        return CompilationErr.Instance;
	        }
	        
	        if(!ValidateIdentifier(identifier, context)) return CompilationErr.Instance;

	        if (!_identifierToItem.TryGetValue(identifier, out var value))
	        {
		        _identifierToItem[identifier] = value = _items.Find(i => string.Equals(i.Name, identifier, StringComparison.CurrentCulture));
	        }

	        if (value == null)
	        {
		        context.LogError(7, $"Could not find '{identifier}' in {this}");
		        return CompilationErr.Instance;
	        }

	        return value as IValue;
        }
        
        public bool AddItem(string identifier, Item item, CompilationContext compilationContext)
        {
	        if (!ValidateIdentifier(identifier, compilationContext)) return false;
			
			if (_identifierToItem.TryGetValue(identifier, out var found))
			{
				compilationContext.LogError(2, $"Cannot add duplicate identifier '{identifier}'");
				return false;
			}

			_identifierToItem.Add(identifier, item);
			return true;
		}

        private bool ValidateIdentifier(string identifier, CompilationContext compilationContext)
        {
	        if (Parser.GloballyReservedIdentifiers.Any(reserved => string.Equals(identifier, reserved, StringComparison.OrdinalIgnoreCase)))
	        {
		        compilationContext.LogError(15, $"'{identifier}' is a reserved identifier");
		        return false;
	        }

	        return true;
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