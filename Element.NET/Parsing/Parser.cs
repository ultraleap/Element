using System.Linq;

namespace Element
{
	using Eto.Parse;
	using Eto.Parse.Parsers;

	public static class ElementAST
	{
		// Expressions - the most basic components which everything else is composed by
		public const string NumberExpression = "num";
		public const string VariableExpression = "var";
		public const string SubExpression = "sub";
		public const string CallExpression = "call";

		// Expression-specific ElementAST elements
		public const string SubExpressionRoot = "subroot";
		public const string SubExpressionName = "subname";
		public const string Callee = "callee";
		public const string CallArguments = "args";
		public const string CallArgument = "arg";

		// Ports - defines syntax for a particular input or output from a function
		public const string Port = "port";
		public const string PortType = "porttype";
		public const string PortName = "portname";

		// Functions - all components for a function
		public const string FunctionName = "name";
		public const string FunctionInputs = "inputs";
		public const string FunctionOutputs = "outputs";
		public const string FunctionBody = "body";

		// Statement - all assignments and function declarations are statements
		public const string AssignmentStatement = "assign";
		public const string TypeStatement = "type";
		public const string Statement = "statement";
	}

	/// <summary>
	/// Provides methods to convert text into Functions
	/// </summary>
	public static class Parser
	{
		private static readonly char[] _identifierAllowedCharacters = {'_'};
		private const char _lineCommentCharacter = '#';

		private static Grammar MakeParser()
		{
				// Literals
				var ws = Terminals.WhiteSpace.Repeat(0);
				var comma = ws.Then(Terminals.Set(','), ws);
				var subExpressionAccess = ws.Then(Terminals.Set('.'), ws);
				var number = new NumberParser
				{
					AllowDecimal = true,
					AllowExponent = true,
					AllowSign = true,
					ValueType = typeof(float)
				};

				var identifier = Terminals.Letter.Or(Terminals.Set(_identifierAllowedCharacters))
					.Then(Terminals.LetterOrDigit.Or(Terminals.Set(_identifierAllowedCharacters)).Repeat(0));

				// Expressions
				var expression = new UnaryParser();
				var subExpression = expression.Named(ElementAST.SubExpressionRoot)
					.Then(subExpressionAccess, identifier.Named(ElementAST.SubExpressionName));
				var arguments = expression.Named(ElementAST.CallArgument).Repeat(0).SeparatedBy(comma);
				var call = expression.Named(ElementAST.Callee)
					.Then(ws, Terminals.Set('('), ws, arguments.Named(ElementAST.CallArguments), ws,
						Terminals.Set(')'));
				expression.Inner = new AlternativeParser(
					number.Named(ElementAST.NumberExpression),
					identifier.Named(ElementAST.VariableExpression),
					subExpression.Named(ElementAST.SubExpression),
					call.Named(ElementAST.CallExpression)
				);

				// Functions
				var portType = ws.Then(Terminals.Literal(":"), ws, identifier.Named(ElementAST.PortType)).Optional();
				var port = identifier.Named(ElementAST.PortName).Then(portType).Named(ElementAST.Port);
				var ports = port.Repeat(0).SeparatedBy(comma);
				var fnInputs = Terminals.Set('(')
					.Then(ws, ports.Named(ElementAST.FunctionInputs), ws, Terminals.Set(')')).Optional();
				var fnOutputs = Terminals.Literal("->").Then(ws, ports.Named(ElementAST.FunctionOutputs)).Or(portType);
				var fnSignature = identifier.Named(ElementAST.FunctionName).Then(ws, fnInputs, ws, fnOutputs, ws);

				// Statements
				var statement = new UnaryParser();
				var body = Terminals.Set('{').Then(ws, statement.Then(ws).Repeat(0).Named(ElementAST.FunctionBody), ws,
					Terminals.Set('}'));
				var assign = Terminals.Set('=').Then(ws, expression.Named(ElementAST.AssignmentStatement), ws,
					Terminals.Set(';'));
				statement.Inner = fnSignature
					.Then(body.Or(assign).Or(Terminals.Set(';').Named(ElementAST.TypeStatement)))
					.Named(ElementAST.Statement);

				var start = ws.Then(statement, ws).Repeat(0);
				start.Until = Terminals.End;

				return new Grammar(start);
		}

		private static string Preprocess(string text)
		{
			using var input = new System.IO.StringReader(text);
			using var output = new System.IO.StringWriter();
			while (true)
			{
				var line = input.ReadLine();
				if (line != null)
				{
					var lineCommentIdx = line.IndexOf(_lineCommentCharacter);
					output.WriteLine(lineCommentIdx >= 0 ? line.Substring(0, lineCommentIdx) : line);
				}
				else
				{
					return output.ToString();
				}
			}
		}

		private static GrammarMatch? Parse(CompilationContext context, string text)
		{
			var match = MakeParser().Match(Preprocess(text));
			if (string.IsNullOrEmpty(match.ErrorMessage)) return match;
			context.LogError(9, match.ErrorMessage);
			return null;
		}

		private static IFunction ToFunction(IScope scope, Match match, CompilationContext context, string source) =>
			match == null
				? Error.Instance
				: (match[ElementAST.FunctionBody] ? true : match[ElementAST.AssignmentStatement] ? true : false)
					? (IFunction) new CustomFunction(scope, match, null, context, source)
					: new CustomType(scope, match, context, source);

		/// <summary>
		/// Parses the given text as Element code, adding each item to the GlobalScope
		/// </summary>
		/// <param name="globalScope">The globalScope to add the parsed items</param>
		/// <param name="context"></param>
		/// <param name="source">Where the source came from</param>
		/// <param name="text">The source code</param>
		public static void AddToGlobalScope(GlobalScope globalScope, CompilationContext context, string source, string text)
		{
			var grammarMatch = Parse(context, text);
			if (grammarMatch == null) return;
			foreach (var func in grammarMatch.Matches.Select(b => ToFunction(globalScope, b, context, source)))
			{
				if (func is INamedType type)
				{
					globalScope.AddType(type, context);
				}
				else
				{
					globalScope.AddFunction((INamedFunction)func, context);
				}
			}
		}

		/// <summary>
		/// Parses the given text as Element code, adding each item to the GlobalScope
		/// </summary>
		/// <param name="globalScope">The globalScope to add the parsed items</param>
		/// <param name="context"></param>
		/// <param name="path">Path to the source file</param>
		public static void AddToGlobalScope(GlobalScope globalScope, CompilationContext context, string path)
		{
			var text = System.IO.File.ReadAllText(path);
			var source = System.IO.Path.GetFileName(path);
			AddToGlobalScope(globalScope, context, source, text);
		}

		/// <summary>
		/// Parses the given text as a single Element function without adding it to a globalScope
		/// </summary>
		/// <param name="scope">The parent globalScope containing external functions</param>
		/// <param name="text">The source code</param>
		/// <param name="context">Compilation context to use for logging</param>
		/// <returns>The parsed function</returns>
		public static IFunction Parse(this IScope scope, string text, CompilationContext context)
			=> ToFunction(scope, Parse(context, text)?.Matches[0], context, string.Empty);
	}
}