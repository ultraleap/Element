namespace Element.Unity
{
	using System;
	using UnityEditor;
	using UnityEngine;

	public class PathView : IDebugView
	{
		static Material lineMaterial;

		static void CreateLineMaterial()
		{
			if (!lineMaterial)
			{
				// Unity has a built-in shader that is useful for drawing
				// simple colored things.
				Shader shader = Shader.Find("Hidden/Internal-Colored");
				lineMaterial = new Material(shader);
				lineMaterial.hideFlags = HideFlags.HideAndDontSave;
				// Turn on alpha blending
				lineMaterial.SetInt("_SrcBlend", (int)UnityEngine.Rendering.BlendMode.SrcAlpha);
				lineMaterial.SetInt("_DstBlend", (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
				// Turn backface culling off
				lineMaterial.SetInt("_Cull", (int)UnityEngine.Rendering.CullMode.Off);
				// Turn off depth writes
				lineMaterial.SetInt("_ZWrite", 0);
			}
		}

		static RenderTexture texture;

		static void CreateTarget()
		{
			if (!texture)
			{
				texture = new RenderTexture(200, 200, 0, RenderTextureFormat.ARGB32);
			}
		}

		private delegate void Path(float u, out float x, out float y, out float z);

		private static readonly Vector3[] Points = new Vector3[200];

		public void OnGUI(string name, IFunction value, CompilerInfo info, Action<string, IFunction> drawOther)
		{
			var at = value.Call(Array.Empty<IFunction>(), "at", info).Compile<Path>();
			CreateLineMaterial();
			CreateTarget();
			var savedC = Graphics.activeColorBuffer;
			var savedD = Graphics.activeDepthBuffer;
			Graphics.SetRenderTarget(texture, 0, CubemapFace.Unknown, 0);
			lineMaterial.SetPass(0);
			GL.Clear(true, true, Color.clear);
			GL.PushMatrix();
			GL.LoadOrtho();
			GL.Begin(GL.LINE_STRIP);
			GL.Color(Color.white);

			System.Threading.Tasks.Parallel.For(0, Points.Length, i =>
			{
				at(i / (float)Points.Length, out var x, out var y, out var z);
				Points[i] = new Vector3(x + 0.5f, y + 0.5f, z);
			});

			foreach (var point in Points)
			{
				GL.Vertex(point);
			}

			GL.Vertex(Points[0]);

			GL.End();
			GL.PopMatrix();
			Graphics.SetRenderTarget(savedC, savedD, 0, CubemapFace.Unknown, 0);
			GUI.color = Color.black;
			GUI.DrawTexture(EditorGUILayout.GetControlRect(GUILayout.Width(200), GUILayout.Height(200)), texture);
			GUI.color = Color.white;
		}

		public bool Supports(IFunction value)
		{
			return ElementContext.Instance.GetContext()
			                     .GetType("Path", null)
			                     .SatisfiedBy(value, new CompilerInfo{Logger = null}) == true;
		}
	}
}