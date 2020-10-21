using System;
using System.Globalization;

namespace Element.CLR
{
    [ElementStructTemplate("TimePoint")]
    public struct TimePoint
    {
        public float whole;
        public float fractional;

        public TimePoint(double time)
        {
            whole = (float)Math.Floor(time);
            fractional = (float)(time - whole);
        }
	
        public TimePoint(float whole, float fractional)
        {
            this.whole = whole;
            this.fractional = fractional;
        }

        public override string ToString() => ((double)whole + fractional).ToString(CultureInfo.InvariantCulture);
    }
}