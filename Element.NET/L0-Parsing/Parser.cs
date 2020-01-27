using System.Collections.Generic;
using System.IO;
using System.Linq;
using Eto.Parse;
using Eto.Parse.Parsers;

namespace Element
{
	/*public static class ElementAST
	{
		// Expressions - the most basic components which everything else is composed by
		public const string LiteralExpression = "lit";
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
		public const string Binding = "bind";
		public const string TypeStatement = "type";
		public const string Declaration = "decl";
	}*/

	public static class SyntaxNodes
	{
		public const string Literal = "lit";
		public const string Identifier = "id";

		public const string ExpressionBody = "exprbody";
		public const string Declaration = "decl";

		public const string Function = "fun";
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
				var ws = Terminals.WhiteSpace.Repeat(0);
				//var comma = ws.Then(Terminals.Set(','), ws);
				//var indexer = Terminals.Set('.');
				var terminal = Terminals.Set(';');
				var literal = new NumberParser
				{
					AllowDecimal = true,
					AllowExponent = true,
					AllowSign = true,
					ValueType = typeof(float)
				};
				var identifier = Terminals.Set('_').Optional()
					.Then(Terminals.Letter.Or(Terminals.Set(_identifierAllowedCharacters)),
						Terminals.LetterOrDigit.Or(Terminals.Set(_identifierAllowedCharacters)).Repeat(0));

				var expression = new UnaryParser
				{
					Inner = new AlternativeParser(
						literal.Named(SyntaxNodes.Literal),
						identifier.Named(SyntaxNodes.Identifier))
				};

				var expressionBody = ws.Then(Terminals.Set('='), ws, expression.Named(SyntaxNodes.ExpressionBody), ws, terminal);
				var declaration = identifier.Named(SyntaxNodes.Declaration);

				var function = declaration.Then(expressionBody).Named(SyntaxNodes.Function);

				var element = function;



				/*// Expressions
				var expression = new UnaryParser();
				var subExpression = expression.Named(ElementAST.SubExpressionRoot)
					.Then(indexer, identifier.Named(ElementAST.SubExpressionName));
				var arguments = expression.Named(ElementAST.CallArgument).Repeat(0).SeparatedBy(comma);
				var call = expression.Named(ElementAST.Callee)
					.Then(ws, Terminals.Set('('), ws, arguments.Named(ElementAST.CallArguments), ws,
						Terminals.Set(')'));
				expression.Inner = new AlternativeParser(
					number.Named(ElementAST.LiteralExpression),
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
				var assign = Terminals.Set('=').Then(ws, expression.Named(ElementAST.Binding), ws,
					Terminals.Set(';'));
				statement.Inner = fnSignature
					.Then(body.Or(assign).Or(Terminals.Set(';').Named(ElementAST.TypeStatement)))
					.Named(ElementAST.Declaration);*/

				var start = ws.Then(element.Optional(), ws).Repeat(0);
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

		private static IValue ToValue(this Match match)
		{
			if (!match.Success) return CompilationErr.Instance;
			return new Literal(match[SyntaxNodes.Declaration].Text, (float)match[SyntaxNodes.ExpressionBody][SyntaxNodes.Literal].Value);
		}

		internal static CallSite MakeCallSite(IFunction function, FileInfo source, Match match)
		{
			var text = ((Eto.Parse.Scanners.StringScanner)match.Scanner).Value;
			var line = 1;
			var column = 1;
			var index = match.Index;
			for (var i = 0; i < index; i++)
			{
				if (text[i] == '\r') {
					line++;
					column = 1;
					if (i+1 < text.Length && text[i+1] == '\n') {
						i++;
					}
				}
				else if (text[i] == '\n') {
					line++;
					column = 1;
				}
				else if (text[i] == '\t') {
					column += 4;
				}
				else
				{
					column++;
				}
			}

			return new CallSite(function, source, line, column);
		}

		/*private static IFunction ToFunction(this Match match, IScope scope,  FileInfo source, CompilationContext context) =>
			match.Success
				? (match[ElementAST.FunctionBody] ? true : match[ElementAST.Binding] ? true : false)
					  ? (IFunction) new CustomFunction(scope, match, null, context, source)
					  : new CustomType(scope, match, context, source)
				: CompilationError.Instance;*/

		/// <summary>
		/// Parses the given file as an Element source file and adds it's contents to a global scope
		/// </summary>
		public static bool ParseFile(this CompilationContext context, FileInfo file) => context.Parse(
			File.ReadAllText(file.FullName)).Matches.Aggregate(true,
			(current, element) => current & context.GlobalScope.Add(element.ToValue(), context));

		/// <summary>
		/// Parses all the given files as Element source files into a global scope
		/// </summary>
		public static IEnumerable<(bool Success, FileInfo FileInfo)> ParseFiles(this CompilationContext context,
		                                                                        IEnumerable<FileInfo> files) =>
			files.Select(file => (context.ParseFile(file), file)).ToArray();

		/*/// <summary>
		/// Parses the given text as a single Element function using a scope without adding it to the scope
		/// </summary>
		public static IFunction ParseText(this CompilationContext context, string text, IScope scope) =>
			context.Parse(text).ToFunction(scope, null, context);*/
	}
}