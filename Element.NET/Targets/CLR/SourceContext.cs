namespace Element.CLR
{
	using System;
	using System.Collections.Generic;
	using System.IO;

	/// <summary>
	/// Element function context which caches source files and provides access to functions declared in them.
	/// </summary>
	public class SourceContext
	{
		public GlobalScope GlobalScope { get; private set; }

		private readonly Dictionary<string, string> _sourceFiles = new Dictionary<string, string>();
		private readonly RootCLRBoundaryMap _boundaryMap;

		public SourceContext(RootCLRBoundaryMap boundaryMap = null)
		{
			_boundaryMap = boundaryMap ?? new RootCLRBoundaryMap();
		}

		public void AddSourceFiles(IEnumerable<FileInfo> sourceFiles)
		{
			foreach (var file in sourceFiles)
			{
				string fileText;
				using (var reader = file.OpenText())
				{
					fileText = reader.ReadToEnd();
				}

				if (!_sourceFiles.ContainsKey(file.Name))
				{
					_sourceFiles[file.Name] = fileText;
				}
				else
				{
					_sourceFiles[file.FullName] = fileText;
				}
			}
		}

		public void Recompile(CompilationContext context)
		{
			GlobalScope = new GlobalScope();
			if (_sourceFiles.Count < 1) return;

			foreach (var file in _sourceFiles)
			{
				Parser.AddToGlobalScope(GlobalScope, context, file.Key, file.Value);
			}

			OnRecompile?.Invoke(GlobalScope, _boundaryMap, context);
		}

		public event Action<GlobalScope, RootCLRBoundaryMap, CompilationContext> OnRecompile;
	}
}