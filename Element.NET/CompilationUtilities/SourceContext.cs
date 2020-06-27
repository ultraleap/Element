using System.Collections.Generic;
using System.IO;
using System.Linq;
using Element.AST;

namespace Element
{
    public class SourceContext : TraceBase
    {
        private SourceContext(CompilationInput compilationInput)
        {
            CompilationInput = compilationInput;
        }
        
        public GlobalScope GlobalScope { get; } = new GlobalScope();
        public CompilationInput CompilationInput { get; }
        

        public static Result<SourceContext> Create(CompilationInput compilationInput)
        {
            var context = new SourceContext(compilationInput);
            return context.LoadPackagesAndExtraSourceFiles().Map(() => context);
        }

        public Result<IValue> EvaluateExpression(string expression) =>
            Parser.Parse<AST.Expression>(expression, this, CompilationInput.NoParseTrace)
                  .Bind(expressionObject =>
                  {
                      expressionObject.InitializeUsingStubDeclaration(expression, GlobalScope);
                      var validationResultBuilder = new ResultBuilder(this);
                      expressionObject.Validate(validationResultBuilder);
                      return validationResultBuilder.ToResult()
                                          .Bind(() => expressionObject.ResolveExpression(GlobalScope, new CompilationContext(this)));
                  });

        public Result<SourceContext> LoadElementSourceFile(FileInfo file) => LoadElementSourceString(new SourceInfo(file.FullName, File.ReadAllText(file.FullName)));

        public Result<SourceContext> LoadElementSourceString(SourceInfo info) =>
            GlobalScope.ContainsSource(info.Name)
                ? Trace(MessageCode.DuplicateSourceFile, $"'{info.Name}' already added")
                : Parser.Parse<SourceBlob>(info.PreprocessedText, this, CompilationInput.NoParseTrace)
                        .Do(sourceScope =>
                        {
                            sourceScope.Initialize(in info, GlobalScope);
                            return GlobalScope.AddSource(info.Name, sourceScope, this);
                        })
                        .Map(() => this);

        /// <summary>
        /// Parses all the given files as Element source files into the source context
        /// </summary>
        public Result<SourceContext> LoadElementSourceFiles(IEnumerable<FileInfo> files) => files.Select(LoadElementSourceFile).Fold().Map(() => this);
        
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
                
                return (Result)LoadElementSourceFiles(CompilationInput.Packages
                                                              .SelectMany(LoadPackage)
                                                              .Concat(preludeSourceFiles)
                                                              .Concat(CompilationInput.ExtraSourceFiles)
                                                              .Distinct(_fileComparer));
            }
        }

        public override MessageLevel Verbosity => CompilationInput.Verbosity;
        public override IReadOnlyCollection<TraceSite>? TraceStack => null;
    }
}