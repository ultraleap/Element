using System.Collections.Generic;
using System.IO;
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

		private static GrammarMatch Parse(this CompilationContext compilationContext, string text)
		{
			var match = MakeParser().Match(Preprocess(text));
			if (!match.Success)
			{
				compilationContext.LogError(9, match.ErrorMessage);
			}

			return match;
		}

		private static IFunction ToFunction(this Match match, IScope scope,  FileInfo source, CompilationContext context) =>
			match.Success
				? (match[ElementAST.FunctionBody] ? true : match[ElementAST.AssignmentStatement] ? true : false)
					  ? (IFunction) new CustomFunction(scope, match, null, context, source)
					  : new CustomType(scope, match, context, source)
				: CompilationError.Instance;

		/// <summary>
		/// Parses the given file as an Element source file and adds it's contents to a global scope
		/// </summary>
		public static CompilationContext ParseFile(this CompilationContext context, FileInfo file) =>
			context.GlobalScope.Add(context.Parse(File.ReadAllText(file.FullName)).ToFunction(context.GlobalScope, file, context), context);

		/// <summary>
		/// Parses all the given files as Element source files into a global scope
		/// </summary>
		public static CompilationContext ParseFiles(this CompilationContext context, IEnumerable<FileInfo> files) =>
			files.Aggregate(context, (ctx, info) => ctx.ParseFile(info));

		/// <summary>
		/// Parses the given text as a single Element function using a scope without adding it to the scope
		/// </summary>
		public static IFunction ParseText(this CompilationContext context, string text, IScope scope) =>
			context.Parse(text).ToFunction(scope, null, context);
	}
}