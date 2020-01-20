using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Ultimately;

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

		private static Option<GrammarMatch> Parse(string text)
		{
			var match = MakeParser().Match(Preprocess(text));
			return string.IsNullOrEmpty(match.ErrorMessage)
				? Optional.None<GrammarMatch>(match.ErrorMessage)
				: Optional.Some(match);
		}

		private static IFunction ToFunction(IScope scope, Match match, CompilationContext context, FileInfo source) =>
			(match[ElementAST.FunctionBody] ? true : match[ElementAST.AssignmentStatement] ? true : false)
				? (IFunction) new CustomFunction(scope, match, null, context, source)
				: new CustomType(scope, match, context, source);

		/// <summary>
		/// Parses the given file as an Element source file, adding it's contents to the given global scope
		/// </summary>
		/// <param name="globalScope">The globalScope to add the parsed items</param>
		/// <param name="context"></param>
		/// <param name="file">Source file</param>
		public static GlobalScope ParseFile(this GlobalScope globalScope, CompilationContext context, FileInfo file) =>
			Parse(File.ReadAllText(file.FullName)).Match(match => globalScope.Add(ToFunction(globalScope, match, context, file), context), error =>
			{
				context.LogError(9, error.Message);
				return globalScope;
			});

		/// <summary>
		/// Parses all the given files as Element source files into the global scope
		/// </summary>
		/// <param name="globalScope">Global scope to add parsed file items into</param>
		/// <param name="context"></param>
		/// <param name="files"></param>
		public static GlobalScope ParseFiles(this GlobalScope globalScope, CompilationContext context, IEnumerable<FileInfo> files) =>
			files.Aggregate(globalScope, (scope, info) => scope.ParseFile(context, info));

		/// <summary>
		/// Parses the given text as a single Element function without adding it to a globalScope
		/// </summary>
		/// <param name="scope">The parent globalScope containing external functions</param>
		/// <param name="text">The source code</param>
		/// <param name="context">Compilation context to use for logging</param>
		/// <returns>The parsed function</returns>
		public static IFunction Parse(this IScope scope, string text, CompilationContext context) =>
			Parse(text).Match(match => ToFunction(scope, match.Matches[0], context, null),
				error => context.LogError(9, error.Message));
	}
}