using System.Collections.Generic;
using System.IO;
using System.Linq;
using Element.AST;

namespace Element
{
    public interface IIntrinsicCache
    {
        Result<TDeclaration> GetIntrinsic<TDeclaration>(Identifier intrinsicIdentifier) where TDeclaration : Declaration;
        void CacheIntrinsicDeclaration(Declaration declaration);
    }
    
    public class SourceContext : IIntrinsicCache, ITrace
    {
        private SourceContext(CompilationInput compilationInput)
        {
            CompilationInput = compilationInput;
        }
        
        internal GlobalScope GlobalScope { get; } = new GlobalScope();
        public CompilationInput CompilationInput { get; }
        
        public CompilerMessage? Trace(MessageCode messageCode, string context)=>
            CompilerMessage.TryGetMessageLevel((int)messageCode, out var level)
            && CompilationInput.Verbosity >= level
                ? new CompilerMessage(messageCode, context)
                : null;

        public static Result<SourceContext> Create(CompilationInput compilationInput)
        {
            var context = new SourceContext(compilationInput);
            return context.LoadPackagesAndExtraSourceFiles().Map(() => context);
        }

        public Result<IValue> EvaluateExpression(string expression) =>
            Parser.Parse(expression, out AST.Expression expressionObject, this, CompilationInput.NoParseTrace)
                  .And(() =>
                  {
                      expressionObject.InitializeUsingStubDeclaration(expression, GlobalScope, this);
                      var resultBuilder = new ResultBuilder(this);
                      expressionObject.Validate(resultBuilder);
                      return resultBuilder.ToResult();
                  })
                  .Bind(() => expressionObject.ResolveExpression(GlobalScope, new CompilationContext(this)));

        public Result<TValue> EvaluateExpressionAs<TValue>(string expression)
            where TValue : class, IValue =>
            EvaluateExpression(expression).Bind(v => v is TValue value
                                                         ? (Result<TValue>)value
                                                         : Trace(MessageCode.InvalidExpression, $"'{v}' is not a '{typeof(TValue)}'"));

        public Result LoadElementSourceFile(FileInfo file) => LoadElementSourceString(new SourceInfo(file.FullName, File.ReadAllText(file.FullName)));

        public Result LoadElementSourceString(SourceInfo info) =>
            GlobalScope.ContainsSource(info.Name)
                ? Trace(MessageCode.DuplicateSourceFile, $"'{info.Name}' already added")
                : Parser.Parse<SourceScope>(info.PreprocessedText, out var sourceScope, this, CompilationInput.NoParseTrace)
                        .And(() =>
                        {
                            sourceScope.InitializeItems(info, GlobalScope, this);
                            return GlobalScope.AddSource(info.Name, sourceScope);
                        });

        /// <summary>
        /// Parses all the given files as Element source files into the source context
        /// </summary>
        public Result LoadElementSourceFiles(IEnumerable<FileInfo> files) => files.Select(LoadElementSourceFile).Fold();
        
        public Result ApplyExtraInput(CompilationInput input)
        {
            if (CompilationInput == input) return Result.Success; // CompilationInput is immutable so this is a no-op
            lock (_syncRoot)
            {
                CompilationInput.Packages = CompilationInput.Packages.Union(input.Packages, _directoryComparer).ToArray();
                CompilationInput.ExtraSourceFiles = CompilationInput.ExtraSourceFiles.Union(input.ExtraSourceFiles, _fileComparer).ToArray();

                return LoadPackagesAndExtraSourceFiles();
            }
        }

        private static readonly LambdaEqualityComparer<DirectoryInfo> _directoryComparer = new LambdaEqualityComparer<DirectoryInfo>((a, b) => a.FullName == b.FullName, info => info.GetHashCode());
        private static readonly LambdaEqualityComparer<FileInfo> _fileComparer = new LambdaEqualityComparer<FileInfo>((a, b) => a.FullName == b.FullName, info => info.GetHashCode());
        private static readonly object _syncRoot = new object();

        private readonly Dictionary<string, Declaration> _intrinsicDeclarations = new Dictionary<string, Declaration>();
        private readonly Dictionary<string, List<Declaration>> _multiplyDefinedIntrinsics = new Dictionary<string, List<Declaration>>();

        public Result<TDeclaration> GetIntrinsic<TDeclaration>(Identifier intrinsicIdentifier)
            where TDeclaration : Declaration
        {
            var result = _intrinsicDeclarations.TryGetValue(intrinsicIdentifier, out var declaration)
                             ? declaration is TDeclaration decl
                                   ? new Result<TDeclaration>(decl)
                                   : Trace(MessageCode.TypeError, $"Found intrinsic '{intrinsicIdentifier}' but it is not a '{typeof(TDeclaration)}'")
                             : Trace(MessageCode.IntrinsicNotFound, $"No intrinsic '{intrinsicIdentifier}' in cache");
            return _multiplyDefinedIntrinsics.TryGetValue(intrinsicIdentifier, out var declarations)
                ? new Result<TDeclaration>(result, Trace(MessageCode.MultipleIntrinsicLocations, $"Intrinsic '{intrinsicIdentifier}' is defined in multiple locations Locations:\n{string.Join("    \\n", declarations.Select(d => d.Location))}"))
                : result;
        }

        void IIntrinsicCache.CacheIntrinsicDeclaration(Declaration declaration)
        {
            if (_intrinsicDeclarations.ContainsKey(declaration.Identifier))
            {
                if (_multiplyDefinedIntrinsics.TryGetValue(declaration.Identifier, out var declarations))
                {
                    declarations.Add(declaration);
                }
                else
                {
                    _multiplyDefinedIntrinsics.Add(declaration.Identifier, new List<Declaration>
                    {
                        _intrinsicDeclarations[declaration.Identifier], // The one we already cached
                        declaration // The second one we just discovered
                    });
                }
            }
            else
            {
                _intrinsicDeclarations[declaration.Identifier] = declaration;
            }
        }
        
        private Result LoadPackagesAndExtraSourceFiles()
        {
            lock (_syncRoot)
            {
                static IEnumerable<FileInfo> LoadPackage(DirectoryInfo packageDirectory) =>
                    packageDirectory.GetFiles("*.ele", SearchOption.AllDirectories);
                
                // TODO: Have prelude/standard library provided by compiler itself rather than found in packages
                IEnumerable<FileInfo> preludeSourceFiles;
                if (CompilationInput.ExcludePrelude) preludeSourceFiles = Enumerable.Empty<FileInfo>();
                else
                {
                    var currentDir = new DirectoryInfo(Directory.GetCurrentDirectory());
                    var prelude = currentDir.GetDirectories("Prelude", SearchOption.AllDirectories);
                    if (prelude.Length == 1)
                    {
                        preludeSourceFiles = LoadPackage(prelude[0]);
                    }
                    else if (prelude.Length > 1)
                    {
                        return Trace(MessageCode.FileAccessError, $"Multiple Prelude packages found: \n{string.Join("    \\n", prelude.Select(d => d.FullName))}");
                    }
                    else
                    {
                        return Trace(MessageCode.FileAccessError, "Prelude package not found");
                    }
                }
                
                return LoadElementSourceFiles(CompilationInput.Packages
                                                              .SelectMany(LoadPackage)
                                                              .Concat(preludeSourceFiles)
                                                              .Concat(CompilationInput.ExtraSourceFiles)
                                                              .Distinct(_fileComparer));
            }
        }
    }
}