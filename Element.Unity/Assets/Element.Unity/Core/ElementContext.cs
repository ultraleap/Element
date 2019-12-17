namespace Element.Unity
{
	using UnityEngine;
	using System.Collections.Generic;
	using System;

	[CreateAssetMenu(menuName = "Element/Context")]
	public class ElementContext : ScriptableObject
	{
		public static ElementContext Instance { get; private set; }

		public ElementContext() => Instance = this;

		public List<TextAsset> Files = new List<TextAsset>();
		public List<UnityEngine.Object> Assets = new List<UnityEngine.Object>();

		private RootContext _context;

		public static event Action NewContext;

		public void Clear()
		{
			_context = null;
			NewContext?.Invoke();
		}

		public RootContext GetContext()
		{
			if (_context == null) {
				_context = new RootContext();
				foreach (var file in Files) {
					if (file == null) { continue; }
					try {
						Parser.AddToContext(_context, file.name, file.text);
					} catch (Exception e) {
						Debug.LogException(e, file);
					}
				}
				Assets.RemoveAll(a => a == null);
				foreach (var obj in Assets) {
					var asset = obj as IElementAsset;
					if ( asset == null) { continue; }
					try {
						Parser.AddToContext(_context, obj.name, asset.ToElement());
					} catch (Exception e) {
						Debug.LogException(e, obj);
					}
				}
			}
			return _context;
		}

		[ContextMenu("Reload")]
		public void Reload()
		{
			Clear();
			GetContext();
		}
	}
}