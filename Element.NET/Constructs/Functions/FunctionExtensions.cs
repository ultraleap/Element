namespace Element
{
	using System;
	using System.Linq;
	using System.Collections.Generic;

	public static class FunctionExtensions
	{
		/// <summary>
		/// Calculates the serialized size of the given Function or Type
		/// </summary>
		/// <param name="value">The value or type in question</param>
		/// <param name="info">Where to log error messages</param>
		/// <returns>The size of the structure in singles, or null if there was a problem</returns>
		public static int? GetSize(this IFunction value, CompilationContext info)
		{
			if (value == Error.Instance)
			{
				return null;
			}

			if (value is SerializableType)
			{
				return 0;
			}

			if (value.IsLeaf() || value.AsExpression(info) != null)
			{
				return 1;
			}

			if (value.Inputs?.Length != 0)
			{
				info.LogError($"Cannot serialize {value} because it has inputs");
				return null;
			}

			if (value.Outputs == null)
			{
				info.LogError($"Cannot serialize {value} because it is not introspectable");
				return null;
			}

			return value.Outputs.Select(o => GetSize(value.Call(o.Name, info), info))
				// NB: An ordinary 'Sum' will treat null as 0
				.Aggregate((int?)0, (a, b) => (a == null || b == null) ? null : a + b);
		}

		/// <summary>
		/// Converts a function into a 1D list of values by recursively evaluating its outputs.
		/// If there are any functions with inputs in the structure, or non-introspectable values, this will fail.
		/// </summary>
		/// <param name="value"></param>
		/// <param name="info"></param>c
		/// <returns>A new array with the serialized values</returns>
		public static Expression[] Serialize(this IFunction value, CompilationContext info)
		{
			var data = new Queue<Expression>();
			Serialize(value, data, info);
			return data.ToArray();
		}

		/// <summary>
		/// See the other overload of Serialize, but this time it takes a Queue rather than making a new array.
		/// </summary>
		/// <param name="value"></param>
		/// <param name="dataOut">The queue to append new values to</param>
		/// <param name="context"></param>
		public static void Serialize(this IFunction value, Queue<Expression> dataOut, CompilationContext context)
		{
			var expr = value.AsExpression(context);
			if (expr != null)
			{
				dataOut.Enqueue(expr);
				return;
			}

			if (value.IsLeaf())
			{
				dataOut.Enqueue(Constant.Zero);
				return;
			}

			if (value is SerializableType)
			{
				return;
			}

			if (value.Inputs?.Length != 0)
			{
				context.LogError($"Cannot serialize {value} because it has inputs");
				return;
			}

			if (value.Outputs == null)
			{
				context.LogError($"{value} doesn't have known outputs");
				return;
			}

			// TODO: Arrays here?
			foreach (var output in value.Outputs)
			{
				Serialize(value.Call(output.Name, context), dataOut, context);
			}
		}

		/// <summary>
		/// Converts a 1D list of inputs into a Function, using the provided function as a template.
		/// </summary>
		/// <returns>The new, deserialized structure mapped to the provided data.</returns>
		public static IFunction Deserialize(this IFunction value, IEnumerable<Expression> data,
		                                    CompilationContext context) =>
			Deserialize(value, new Queue<Expression>(data).Dequeue, context);

		/// <summary>
		/// See the other overload of Deserialize, but this time it takes a delegate to generate new values
		/// rather than a fixed list.
		/// </summary>
		/// <returns>The new, deserialized structure mapped to the provided data.</returns>
		public static IFunction Deserialize(this IFunction function, Func<Expression> data, CompilationContext context)
		{
			if (function.IsLeaf())
			{
				return data();
			}

			if (function is SerializableType)
			{
				return function;
			}

			if (function?.Inputs == null)
			{
				context.LogError("ELE0001", $"Cannot deserialize `{function}` outputs because it's input ports have no defined type");
			}

			return new DeserializedStructure(function, data, new CompilationContext());
		}

		public static IFunction Call(this IFunction function, IFunction[] arguments, string output,
			CompilationContext info, CallSite? callSite = null)
		{
			if (function == null) throw new ArgumentNullException(nameof(function));
			if (callSite.HasValue)
				info.Push(callSite.Value);
			var retval = function.CallInternal(arguments, output, info);
			if (callSite.HasValue)
				info.Pop();
			return retval;
		}

		/// <summary>
		/// Calls a function with no arguments
		/// </summary>
		public static IFunction Call(this IFunction function, string output,
			CompilationContext context, CallSite? callSite = null) =>
			function.Call(Array.Empty<IFunction>(), output, context, callSite);

		/// <summary>
		/// Calls a function with arguments, but without specifying an output
		/// </summary>
		/// <returns>If the function has only one output it returns it, otherwise it returns a structure
		/// with all the function's outputs.</returns>
		public static IFunction Call(this IFunction function, IFunction[] arguments,
			CompilationContext context, CallSite? callSite = null)
		{
			if (function == null) throw new ArgumentNullException(nameof(function));
			if (!(context.Debug && !(function is IType))
				&& function.Outputs?.Length == 1
				&& function.Outputs[0].Name == "return")
			{
				return function.Call(arguments, function.Outputs[0].Name, context, callSite);
			}

			if (function.CheckArguments(arguments, null, context) != null)
			{
				return Error.Instance;
			}

			return new CalledFunction(function, arguments, callSite).ResolveReturns(context, callSite);
		}

		/// <summary>
		/// Checks if the given value is a Leaf (i.e. a single number)
		/// </summary>
		public static bool IsLeaf(this IFunction function) => function.Inputs?.Length == 0 && function.Outputs?.Length == 0;

		public static Expression AsExpression(this IFunction function, CompilationContext info) =>
			(function as IEvaluable)?.AsExpression(info);

		/// <summary>
		/// Checks that the proposed arguments to a function are valid.
		/// </summary>
		/// <param name="function">The function about to be called</param>
		/// <param name="arguments">Ordered list of proposed inputs</param>
		/// <param name="output">The proposed output</param>
		/// <param name="info">The place to log messages on failure</param>
		/// <returns>Null on success, or Abort on failure (this allows using the null-coalesce operator)</returns>
		public static IFunction CheckArguments(this IFunction function, IFunction[] arguments, string output,
		                                       CompilationContext info)
		{
			var success = true;
			var inputs = function.Inputs;
			if (inputs != null)
			{
				if (arguments.Length != inputs.Length)
				{
					info.LogError($"{function} Expected {inputs.Length} arguments but got {arguments.Length}");
					success = false;
				}
				else
				{
					for (var i = 0; i < inputs.Length; i++)
					{
						var arg = inputs[i].Type.SatisfiedBy(arguments[i], info);
						if (arg == false)
						{
							info.LogError(
								$"{function} Input {i} ({inputs[i]}) not satisfied by {arguments[i]} (see previous errors)");
						}
						if (arg != true)
						{
							success = false;
						}
					}
				}
			}

			if (output != null && function.Outputs != null && Array.FindIndex(function.Outputs, p => p.Name == output) < 0)
			{
				info.LogError($"Couldn't find output `{output}` in {function}");
				success = false;
			}

			if (arguments.Any(a => a is Error)) {
				return Error.Instance;
			}

			return success ? null : Error.Instance;
		}

		/// <summary>
		/// If a function has no inputs and one output, this function will iteratively
		/// call these outputs until this is no longer the case. This avoids having to specify 'return' outputs
		/// and suchlike in the source code.
		/// </summary>
		public static IFunction ResolveReturns(this IFunction function, CompilationContext info, CallSite? callSite)
		{
			while (function?.Inputs?.Length == 0
				&& function.Outputs?.Length == 1
				&& function.Outputs[0].Name == "return")
			{
				if (info.Debug && !(function is IType))
				{
					function = new ReturnWrapper(function, info, callSite);
				}
				else
				{
					function = function.Call(function.Outputs[0].Name, info);
				}
			}

			return function;
		}

		private class DeserializedStructure : IFunction
		{
			public DeserializedStructure(IFunction structure, Func<Expression> data, CompilationContext context)
			{
				if (structure.Inputs.Length > 0)
				{
					context.LogError("ELE0001", $"Cannot deserialize {structure} because it has inputs");
				}

				Outputs = structure.Outputs;
				_outputValues = Outputs.Select(o => structure.Call(o.Name, context).Deserialize(data, context)).ToArray();
			}

			public PortInfo[] Inputs => Array.Empty<PortInfo>();
			public PortInfo[] Outputs { get; }
			private readonly IFunction[] _outputValues;

			public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
			{
				return this.CheckArguments(arguments, output, context) ?? _outputValues[Array.FindIndex(Outputs, p => p.Name == output)];
			}
		}

		/// <summary>
		/// Represents a called function with multiple outputs where the output hasn't been selected yet.
		/// </summary>
		private class CalledFunction : IFunction, IEvaluable, IDebuggable
		{
			public CalledFunction(IFunction surrogate, IFunction[] args, CallSite? callSite)
			{
				if (surrogate is CalledFunction)
				{
					throw new InternalCompilerException("Tried to call a function without specifying an output, but it has multiple outputs");
				}

				_surrogate = surrogate;
				_args = args;
				_callSite = callSite;
			}

			private readonly IFunction[] _args;
			private readonly IFunction _surrogate;
			private readonly CallSite? _callSite;

			public override string ToString() => $"{_surrogate}()";
			public PortInfo[] Inputs => Array.Empty<PortInfo>();
			public PortInfo[] Outputs => _surrogate.Outputs;
			public string[] IntermediateValues => (_surrogate as IDebuggable)?.IntermediateValues ?? Array.Empty<string>();

			public string DebugName =>
				(_surrogate as IDebuggable)?.DebugName ?? (_surrogate as INamedItem)?.Name ?? "unnamed";

			public Expression AsExpression(CompilationContext info) =>
				Outputs?.Length == 1
					? this.CallInternal(Array.Empty<IFunction>(), _surrogate.Outputs[0].Name, info).AsExpression(info)
					: null;

			private readonly Dictionary<string, IFunction> _cache = new Dictionary<string, IFunction>();
			private readonly Dictionary<string, IFunction> _debugCache = new Dictionary<string, IFunction>();

			public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
			{
				if (this.CheckArguments(arguments, output, context) != null)
				{
					return Error.Instance;
				}

				if (!_cache.TryGetValue(output, out var found))
				{
					_cache.Add(output, found = _surrogate.Call(_args, output, context, _callSite));
				}

				return found;
			}

			public IFunction CompileIntermediate(IFunction[] arguments, string name, CompilationContext context)
			{
				if (_debugCache.TryGetValue(name, out var found))
				{
					return found;
				}

				found = ((IDebuggable)_surrogate).CompileIntermediate(_args, name, context);
				// If the surrogate returns a function directly, we need to Call it with our own arguments
				if (arguments.Length > 0 && _surrogate.Outputs?.Length == 1 && name == _surrogate.Outputs[0].Name)
				{
					return new CalledFunction(found, arguments, _callSite);
				}

				if (this.CheckArguments(arguments, null, context) != null)
				{
					return Error.Instance;
				}

				_debugCache.Add(name, found);

				return found;
			}
		}

		/// <summary>
		/// Preserves function which creates a return value. Only used in debug mode.
		/// </summary>
		private class ReturnWrapper : IFunction, IDebuggable, IEvaluable
		{
			public ReturnWrapper(IFunction surrogate, CompilationContext info, CallSite? callSite)
			{
				_callSite = callSite;
				_surrogate = surrogate;
				_returnValue = surrogate.Call(Array.Empty<IFunction>(), surrogate.Outputs[0].Name, info);
			}

			public override string ToString() => $"{_returnValue} <- {_surrogate}";

			private readonly IFunction _surrogate;
			private readonly IFunction _returnValue;
			private readonly CallSite? _callSite;

			public PortInfo[] Inputs => _returnValue.Inputs;
			public PortInfo[] Outputs => _returnValue.Outputs;
			public string[] IntermediateValues => (_surrogate as IDebuggable)?.IntermediateValues ?? Array.Empty<string>();

			public string DebugName =>
				(_surrogate as IDebuggable)?.DebugName ?? (_surrogate as INamedItem)?.Name ?? "unnamed";

			public Expression AsExpression(CompilationContext info) => _returnValue.AsExpression(info);

			public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context) =>
				_returnValue.Call(arguments, output, context);

			public IFunction CompileIntermediate(IFunction[] arguments, string name, CompilationContext context)
			{
				if (arguments.Length > 0 && _surrogate.Inputs.Length == 0)
				{
					var found = (_surrogate as IDebuggable)?.CompileIntermediate(Array.Empty<IFunction>(), name, context);
					if (found.Inputs?.Length == arguments.Length)
					{
						return new CalledFunction(found, arguments, _callSite);
					}
				}

				return (_surrogate as IDebuggable)?.CompileIntermediate(arguments, name, context)
					?? throw new ArgumentException($"Name {name} not found in {_surrogate}");
			}
		}

		public static IFunction AsMethod(this IFunction classInstance, IFunction memberFunction, CallSite? callSite, CompilationContext context)
		{
			if (classInstance == null) throw new ArgumentNullException(nameof(classInstance));
			if (memberFunction == null) throw new ArgumentNullException(nameof(memberFunction));
			if (!(context.Debug) && memberFunction.Outputs?.Length == 1)
			{
				return memberFunction.Call(new []{classInstance}, memberFunction.Outputs[0].Name, context, callSite);
			}

			return new Method(memberFunction, classInstance, callSite).ResolveReturns(context, callSite);
		}

		private class Method : IFunction, IEvaluable, IDebuggable
		{
			public Method(IFunction surrogate, IFunction classInstance, CallSite? callSite)
			{
				if (surrogate is Method)
				{
					throw new InternalCompilerException("Method wrapping a method - this should be impossible");
				}

				_surrogate = surrogate;
				_classInstance = classInstance;
				_callSite = callSite;
				Inputs = _surrogate.Inputs.Skip(1).ToArray();
			}

			private readonly IFunction _classInstance;
			private readonly IFunction _surrogate;
			private readonly CallSite? _callSite;

			public override string ToString() => $"{_classInstance}.[{_surrogate}]";
			public PortInfo[] Inputs { get; }
			public PortInfo[] Outputs => _surrogate.Outputs;
			public string[] IntermediateValues => (_surrogate as IDebuggable)?.IntermediateValues ?? Array.Empty<string>();

			public string DebugName =>
				(_surrogate as IDebuggable)?.DebugName ?? (_surrogate as INamedItem)?.Name ?? "unnamed";

			public Expression AsExpression(CompilationContext info) =>
				Outputs?.Length == 1 && Inputs.Length == 0
					? this.CallInternal(Array.Empty<IFunction>(), _surrogate.Outputs[0].Name, info).AsExpression(info)
					: null;

			private readonly Dictionary<string, IFunction> _cache = new Dictionary<string, IFunction>();
			private readonly Dictionary<string, IFunction> _debugCache = new Dictionary<string, IFunction>();

			public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
			{
				var completeArgs = new[] {_classInstance}.Concat(arguments).ToArray();
				if (this.CheckArguments(arguments, output, context) != null)
				{
					return Error.Instance;
				}

				if (!_cache.TryGetValue(output, out var found))
				{
					found = _surrogate.Call(completeArgs, output, context, _callSite);
					if (Inputs.Length == 0) {
						_cache.Add(output, found);
					}
				}

				return found;
			}

			public IFunction CompileIntermediate(IFunction[] arguments, string name, CompilationContext context)
			{
				if (_debugCache.TryGetValue(name, out var found))
				{
					return found;
				}

				var completeArgs = new[] {_classInstance}.Concat(arguments).ToArray();
				found = ((IDebuggable)_surrogate).CompileIntermediate(completeArgs, name, context);
				// If the surrogate returns a function directly, we need to Call it with our own arguments
				if (arguments.Length > 0 && _surrogate.Outputs?.Length == 1 && name == _surrogate.Outputs[0].Name)
				{
					return new CalledFunction(found, arguments, _callSite);
				}

				if (this.CheckArguments(arguments, null, context) != null)
				{
					return Error.Instance;
				}

				if (Inputs.Length == 0) {
					_debugCache.Add(name, found);
				}

				return found;
			}
		}

		public static IFunction Flatten(this IFunction function, string output, CompilationContext info)
		{
			if (function.Inputs == null) {
				throw new ArgumentException($"{function} cannot be flattened");
			}
			return new FlattenedFunction(function, output, info);
		}

		/// <summary>
		/// Represents a function created by hoisting all of a functions  function.
		/// e.g. f(a) --> g(b) --> r can be flattened to fg(a, b) --> r
		/// This process is formally known as Uncurrying (https://en.wikipedia.org/wiki/Currying).
		/// </summary>
		private class FlattenedFunction : IFunction, IDebuggable
		{
			public FlattenedFunction(IFunction surrogate, string output, CompilationContext info)
			{
				this.surrogate = surrogate;
				surrogateName = output;
				var value = surrogate.Call(surrogate.Inputs.Select(p => p.Type).ToArray(), info).Call(output, info);
				Inputs = surrogate.Inputs.Concat(value.Inputs).ToArray();
			}

			public PortInfo[] Inputs { get; }
			public PortInfo[] Outputs { get; } = {new PortInfo {Name = "return", Type = AnyType.Instance}};
			public string[] IntermediateValues => (surrogate as IDebuggable)?.IntermediateValues;
			public string DebugName => (surrogate as IDebuggable)?.DebugName;
			private readonly string surrogateName;
			private readonly IFunction surrogate;

			public IFunction CallInternal(IFunction[] arguments, string output, CompilationContext context)
			{
				if (this.CheckArguments(arguments, output, context) != null)
				{
					return Error.Instance;
				}

				var value = surrogate.Call(arguments.Take(surrogate.Inputs.Length).ToArray(), context)
				                     .Call(surrogateName, context);
				return value.Call(arguments.Skip(surrogate.Inputs.Length).ToArray(), context);
			}

			public IFunction CompileIntermediate(IFunction[] arguments, string name, CompilationContext context)
			{
				return (surrogate as IDebuggable)?.CompileIntermediate(arguments, name, context);
			}
		}

		/// <summary>
		/// Invokes an action recursively on each Debug Value of a function. This can be used to draw a debug tree view
		/// </summary>
		/// <param name="function"></param>
		/// <param name="arguments"></param>
		/// <param name="name">The top level name of the function</param>
		/// <param name="nestedness">A value that is incremented as the tree is descended, and decremented as we go back up to the root</param>
		/// <param name="info"></param>
		/// <param name="action">Invoked for each value, passed the item's path as a string and value as IFunction</param>
		/// <param name="recursePredicate">Passed the same values as for 'action', but used to determine whether to descend into it</param>
		public static void RecursiveAction(this IFunction function, IFunction[] arguments, string name,
		                                   ref int nestedness, CompilationContext info, Action<string, IFunction> action,
		                                   Predicate<(string, IFunction)> recursePredicate)
		{
			action(name, function);

			if (function.Inputs?.Length != arguments.Length)
			{
				info.LogError($"Can't do {function} because we have {arguments.Length} arguments");
			}

			if (function.Inputs?.Length == arguments.Length && recursePredicate((name, function)))
			{
				nestedness++;

				void Recurse(string driver, IFunction fn, bool b, ref int recursionLevel) =>
					RecursiveAction(fn, driver == "return" // TODO: Replace with IDebuggable.IsReturn(string)?
							? arguments
							: Array.Empty<IFunction>(), b ? name : $"{name}.{driver}", ref recursionLevel, info, action,
						recursePredicate);

				if (function is IDebuggable ctx && ctx.IntermediateValues.Length > 0)
				{
					foreach (var debugValue in ctx.IntermediateValues)
					{
						Recurse(debugValue, ctx.CompileIntermediate(arguments, debugValue, info), ctx.IntermediateValues.Length == 1,
							ref nestedness);
					}
				}
				else
				{
					foreach (var output in function.Outputs)
					{
						Recurse(output.Name, function.Call(arguments, output.Name, info), function.Outputs.Length == 1,
							ref nestedness);
					}
				}

				nestedness--;
			}
		}

		public static bool IsNamespace(this IFunction func) => (func as CustomFunction)?.IsNamespace == true;
	}
}