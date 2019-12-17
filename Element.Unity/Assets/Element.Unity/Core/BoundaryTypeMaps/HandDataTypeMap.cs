namespace Element.Unity
{
	using UnityEngine;

	[BoundaryTypeMap]
	public class HandDataTypeMap : BoundaryTypeMap
	{
		private void FillFloat(float f, ref int index, float[] array) => array[index++] = f;

		private void FillVector3(System.Numerics.Vector3 v3, ref int index, float[] array)
		{
			FillFloat(v3.X, ref index, array);
			FillFloat(v3.Z, ref index, array);
			FillFloat(v3.Y, ref index, array);
		}

		private void FillFingerData(FingerData fd, ref int index, float[] array)
		{
			FillVector3(fd.Metacarpal, ref index, array);
			FillVector3(fd.Proximal, ref index, array);
			FillVector3(fd.Distal, ref index, array);
			FillVector3(fd.Tip, ref index, array);
		}

		public override bool SupportsType(IType type, int size) => size > 16;

		public override void Evaluate(IType type, Vector4 value, Object objValue, float[] output, int index, int size)
		{
			var handDataProvider = objValue as HandDataProvider;
			var handData = handDataProvider?.Current ?? default;

			FillFloat(handData.Detected ? 1f : 0f, ref index, output);
			FillFloat(handData.IsRight ? 1f : 0f, ref index, output);
			FillFloat(handData.Width, ref index, output);
			FillVector3(handData.Position, ref index, output);
			FillVector3(handData.Normal, ref index, output);
			FillVector3(handData.Direction, ref index, output);
			FillFingerData(handData.Thumb, ref index, output);
			FillFingerData(handData.Index, ref index, output);
			FillFingerData(handData.Middle, ref index, output);
			FillFingerData(handData.Ring, ref index, output);
			FillFingerData(handData.Pinky, ref index, output);
		}
	}
}