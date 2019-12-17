namespace Element.Unity
{
	using UnityEngine;

	[BoundaryTypeMap]
	public class ConstantTypeMap : BoundaryTypeMap
	{
		public override void Evaluate(IType type, Vector4 value, Object objValue, float[] output, int index, int size)
		{
			WriteVector(value, output, index, size);
		}

		public override bool SupportsType(IType type, int size) => size < 4;
	}

	[BoundaryTypeMap]
	public class TimeValue : BoundaryTypeMap
	{
		public override void Evaluate(IType type, Vector4 value, Object objValue, float[] output, int index, int size)
		{
			output[index] = Time.time;
		}

		public override bool SupportsType(IType type, int size) => size == 1;
	}
}