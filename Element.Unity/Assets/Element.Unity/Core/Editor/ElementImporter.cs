namespace Element.Unity
{
	using UnityEditor.Experimental.AssetImporters;
	using System.IO;
	using System.Linq;
	using UnityEngine;
	using UnityEditor;

	[ScriptedImporter(1, "ele")]
	public class ElementImporter : ScriptedImporter
	{
		public override void OnImportAsset(AssetImportContext ctx)
		{
			var text = new TextAsset(File.ReadAllText(ctx.assetPath));
			ctx.AddObjectToAsset(Path.GetFileNameWithoutExtension(ctx.assetPath), text);
		}
	}

	public class ElementPostprocessor : AssetPostprocessor
	{
		static void OnPostprocessAllAssets(string[] importedAssets,
			string[] deletedAssets, string[] movedAssets, string[] movedFromAssetPaths)
		{
			var eleCtx = AssetDatabase.FindAssets("t:ScriptableObject")
				.Select(AssetDatabase.GUIDToAssetPath)
				.Select(AssetDatabase.LoadAssetAtPath<ElementContext>)
				.FirstOrDefault(o => o != null);
			if (eleCtx)
			{
				eleCtx.Clear();
				// TODO: Potential bug - This may not be sufficient to keep the references!
				eleCtx.Files.AddRange(importedAssets
					.Where(p => Path.GetExtension(p) == ".ele")
					.Select(AssetDatabase.LoadAssetAtPath<TextAsset>)
					.Where(f => !eleCtx.Files.Contains(f)));
				eleCtx.Files.RemoveAll(t => t == null);
			}
		}
	}
}