using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Element.AST;

namespace Element
{
    public class SourceContext : ILogger
    {
        private SourceContext(CompilationInput compilationInput)
        {
            CompilationInput = compilationInput;
        }
        
        public GlobalScope GlobalScope { get; } = new GlobalScope();
        public CompilationInput CompilationInput { get; }

        public static bool TryCreate(CompilationInput compilationInput, out SourceContext sourceContext)
        {
            sourceContext = new SourceContext(compilationInput);
            return sourceContext.LoadPackagesAndExtraSourceFiles();
        }

        private readonly Dictionary<IIntrinsic, Declaration> _cachedIntrinsicDeclarations = new Dictionary<IIntrinsic, Declaration>();
        
        public TDeclaration? GetIntrinsicsDeclaration<TDeclaration>(IIntrinsic intrinsic, CompilationContext compilationContext) where TDeclaration : Declaration
        {
            lock (_syncRoot)
            {
                if (!_cachedIntrinsicDeclarations.TryGetValue(intrinsic, out var declaration))
                {
                    IScope currentScope = GlobalScope;
                    foreach (var id in intrinsic.Location.Split('.'))
                    {
                        declaration = currentScope?[new Identifier(id), false, compilationContext] as Declaration;
                        currentScope = declaration?.Child;
                    }

                    _cachedIntrinsicDeclarations[intrinsic] = declaration;
                }

                return declaration switch
                {
                    TDeclaration d => (IValue) d,
                    {} => LogError(4, $"Found declaration '{intrinsic.Location}' but it is not a {nameof(TDeclaration)}"),
                    _ => LogError(7, $"Couldn't find '{intrinsic.Location}'")
                } as TDeclaration;
            }
        }
        
        public IValue EvaluateExpression(string expression, out CompilationContext compilationContext)
        {
            compilationContext = MakeCompilationContext(out compilationContext);
            var success = Parser.Parse(expression, out AST.Expression expressionObject, this, CompilationInput.NoParseTrace);
            expressionObject.InitializeUsingStubDeclaration(expression, compilationContext);
            success &= expressionObject.Validate(this);
            return success
                       ? expressionObject.ResolveExpression(GlobalScope, compilationContext)
                       : CompilationError.Instance;
        }
        
        public TValue? EvaluateExpressionAs<TValue>(string expression, out CompilationContext compilationContext)
            where TValue : class, IValue
        {
            var result = EvaluateExpression(expression, out compilationContext);
            if (result is TValue value) return value;
            compilationContext.LogError(16, $"'{result}' is not a '{typeof(TValue)}'");
            return null;
        }

        public bool LoadElementSourceString(SourceInfo info) => LoadElementSourceString(info, true);
        public bool LoadElementSourceFile(FileInfo file) => LoadElementSourceFile(file, true);

        private bool LoadElementSourceFile(FileInfo file, bool validate) =>
            LoadElementSourceString(new SourceInfo(file.FullName, Parser.Preprocess(File.ReadAllText(file.FullName))), validate);
        private bool LoadElementSourceString(SourceInfo info, bool validate)
        {
            var success = Parser.Parse<SourceScope>(info.Text, out var sourceScope, this, CompilationInput.NoParseTrace);
            if (success)
            {
                sourceScope.InitializeItems(info, GlobalScope);
                GlobalScope[info.Name] = sourceScope;
                if (validate)
                {
                    success &= GlobalScope.ValidateScope(this);
                }
            }

            return success;
        }

        /// <summary>
        /// Parses all the given files as Element source files into the source context
        /// </summary>
        public (bool OverallSuccess, IEnumerable<(bool Success, FileInfo FileInfo)> Results) LoadElementSourceFiles(IEnumerable<FileInfo> files)
        {
            (bool Success, FileInfo File)[] fileResults = files.Where(file => GlobalScope[file.FullName] == null).Select(file => (LoadElementSourceFile(file, false), file)).ToArray();
            var overallSuccess = fileResults.All(fr => fr.Success) && GlobalScope.ValidateScope(this);
            return (overallSuccess, fileResults);
        }
        
        public CompilationError LogError(int? messageCode, string context)
        {
            var msg = MakeMessage(messageCode, context);
            if (!msg.MessageLevel.HasValue || msg.MessageLevel.Value >= CompilationInput.Verbosity)
            {
                CompilationInput.LogCallback?.Invoke(msg);
            }

            return CompilationError.Instance;
        }

        public void Log(string message)
        {
            var msg = MakeMessage(null, message);
            if (!msg.MessageLevel.HasValue || msg.MessageLevel.Value >= CompilationInput.Verbosity)
            {
                CompilationInput.LogCallback?.Invoke(msg);
            }
        }

        private CompilerMessage MakeMessage(int? messageCode, string context) => !messageCode.HasValue
            ? new CompilerMessage(null, null, context, null)
            : new CompilerMessage(messageCode.Value, CompilerMessage.TryGetMessageLevel(messageCode.Value, out var level) ? level : MessageLevel.Information, context, null);

        private static readonly LambdaEqualityComparer<DirectoryInfo> _directoryComparer = new LambdaEqualityComparer<DirectoryInfo>((a, b) => a.FullName == b.FullName, info => info.GetHashCode());
        private static readonly LambdaEqualityComparer<FileInfo> _fileComparer = new LambdaEqualityComparer<FileInfo>((a, b) => a.FullName == b.FullName, info => info.GetHashCode());

        private static readonly object _syncRoot = new object();

        public CompilationContext MakeCompilationContext(out CompilationContext context) => context = new CompilationContext(this);

        public bool ApplyExtraInput(CompilationInput input)
        {
            if (CompilationInput == input) return true;
            lock (_syncRoot)
            {
                CompilationInput.Packages = CompilationInput.Packages.Union(input.Packages, _directoryComparer).ToArray();
                CompilationInput.ExtraSourceFiles = CompilationInput.ExtraSourceFiles.Union(input.ExtraSourceFiles, _fileComparer).ToArray();

                return LoadPackagesAndExtraSourceFiles();
            }
        }

        private bool LoadPackagesAndExtraSourceFiles()
        {
            lock (_syncRoot)
            {
                _cachedIntrinsicDeclarations.Clear();
                return LoadElementSourceFiles(CompilationInput.Packages
                        .Prepend(CompilationInput.ExcludePrelude ? null : new DirectoryInfo("Prelude"))
                        .SelectMany(directory => directory?.GetFiles("*.ele", SearchOption.TopDirectoryOnly) ?? Array.Empty<FileInfo>())
                        .Concat(CompilationInput.ExtraSourceFiles)
                        .ToArray())
                    .OverallSuccess;
            }
        }
    }
}