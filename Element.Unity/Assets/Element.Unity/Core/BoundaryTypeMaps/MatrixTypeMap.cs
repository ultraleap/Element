namespace Element.Unity
{
	using UnityEngine;

	[BoundaryTypeMap]
	public class MatrixTypeMap : BoundaryTypeMap
	{
		private static readonly Matrix4x4 _convertYZ =
			new Matrix4x4(
				new Vector4(1, 0, 0, 0),
				new Vector4(0, 0, 1, 0),
				new Vector4(0, 1, 0, 0),
				new Vector4(0, 0, 0, 1));

		public override void Evaluate(IType type, Vector4 value, Object objValue, float[] output, int index, int size)
		{
			var transform = objValue as Transform;
			var matrix = transform == null ? Matrix4x4.identity : transform.localToWorldMatrix;
			matrix = _convertYZ * matrix;
			for (int i = 0; i < 4; i++)
			{
				WriteVector(matrix.GetColumn(i), output, index + i * 4, 4);
			}
		}

		public override bool SupportsType(IType type, int size) => size == 16;
	}
}