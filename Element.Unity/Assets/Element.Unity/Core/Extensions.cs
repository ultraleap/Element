namespace Element.Unity
{
	using System.Collections.Generic;
	using System.Numerics;

	public static class Extensions
	{
		public static UnityEngine.Vector3 AsUnityVector(this Vector3 v) => new UnityEngine.Vector3(v.X, v.Y, v.Z);

		public static bool SetPresent<T>(this HashSet<T> set, T value, bool presence) => presence ? set.Add(value) : set.Remove(value);
	}
}