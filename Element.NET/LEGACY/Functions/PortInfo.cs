namespace Element
{
	/// <summary>
	/// Information about a particular port (a port is an input or output to a function)
	/// </summary>
	public struct PortInfo
	{
		/// <summary>
		/// The name of the port, as written
		/// </summary>
		public string Name { get; set; }

		/// <summary>
		/// The port's type. Where inferred, this should be the AnyType (not null)
		/// </summary>
		public IType Type { get; set; }

		public override string ToString() => $"{Name}:{Type}";
	}
}