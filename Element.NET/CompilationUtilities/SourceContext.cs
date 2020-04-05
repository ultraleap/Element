using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Element.AST;

namespace Element
{
    public class SourceContext : Context
    {
        private SourceContext(CompilationInput compilationInput) : base(compilationInput) { }
        
        public GlobalScope GlobalScope { get; } = new GlobalScope();

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
                        currentScope = declaration?.ChildScope;
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

        protected override CompilerMessage MakeMessage(int? messageCode, string context = default)=> !messageCode.HasValue
            ? new CompilerMessage(null, null, context, null)
            : new CompilerMessage(messageCode.Value, CompilerMessage.GetMessageLevel(messageCode.Value), context, null);

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
                return this.ParseFiles(CompilationInput.Packages
                        .Prepend(CompilationInput.ExcludePrelude ? null : new DirectoryInfo("Prelude"))
                        .SelectMany(directory =>
                            directory?.GetFiles("*.ele", SearchOption.AllDirectories) ?? Array.Empty<FileInfo>())
                        .Concat(CompilationInput.ExtraSourceFiles)
                        .ToArray())
                    .OverallSuccess;
            }
        }
    }
}