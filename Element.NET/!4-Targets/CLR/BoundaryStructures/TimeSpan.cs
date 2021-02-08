using System;
using System.Globalization;

namespace Element.CLR
{
    [ElementBoundaryStruct("TimeSpan")]
    public struct TimeSpan
    {
        public float integer;
        public float fractional;

        public TimeSpan(double time)
        {
            integer = (float)Math.Floor(time);
            fractional = (float)(time - integer);
        }
	
        public TimeSpan(float integer, float fractional)
        {
            this.integer = integer;
            this.fractional = fractional;
        }

        public override string ToString() => ((double)integer + fractional).ToString(CultureInfo.InvariantCulture);
    }
}