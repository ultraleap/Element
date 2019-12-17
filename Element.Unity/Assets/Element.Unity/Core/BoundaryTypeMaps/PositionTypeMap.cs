namespace Element.Unity
{
	using UnityEngine;

	[BoundaryTypeMap]
	public class PositionTypeMap : BoundaryTypeMap
	{
		public override void Evaluate(IType type, Vector4 value, Object objValue, float[] output, int index, int size)
		{
			var transform = objValue as Transform;
			var pos = transform == null ? Vector3.zero : transform.position;
			WriteVector(new Vector4(pos.x, pos.z, pos.y, 1), output, index, size);
		}

		public override bool SupportsType(IType type, int size) => size == 3 || size == 4;
	}
}