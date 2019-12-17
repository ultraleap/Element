namespace Element.Unity
{
	using System;
	using UnityEngine;
	using Object = UnityEngine.Object;

	[AttributeUsage(AttributeTargets.Class)]
	public class BoundaryTypeMapAttribute : Attribute { }

	public abstract class BoundaryTypeMap
	{
		public abstract bool SupportsType(IType type, int size);
		public abstract void Evaluate(IType type, Vector4 value, Object objValue, float[] output, int index, int size);

		protected static void WriteVector(Vector4 value, float[] output, int index, int size)
		{
			switch (size)
			{
				case 4: output[index + 3] = value.w; goto case 3;
				case 3: output[index + 2] = value.z; goto case 2;
				case 2: output[index + 1] = value.y; goto case 1;
				case 1: output[index + 0] = value.x; break;
				default: throw new NotSupportedException();
			}
		}
	}
}