using System;
using System.Globalization;

namespace Element.CLR
{
    [ElementStructTemplate("TimeSpan")]
    public struct TimeSpan
    {
        public float whole;
        public float fractional;

        public TimeSpan(double time)
        {
            whole = (float)Math.Floor(time);
            fractional = (float)(time - whole);
        }
	
        public TimeSpan(float whole, float fractional)
        {
            this.whole = whole;
            this.fractional = fractional;
        }

        public override string ToString() => ((double)whole + fractional).ToString(CultureInfo.InvariantCulture);
    }
}