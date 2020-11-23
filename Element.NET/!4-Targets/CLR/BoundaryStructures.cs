using System;
using System.Globalization;
using System.Numerics;

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

    [ElementBoundaryStruct("Matrix3x3")]
    public struct Matrix3x3
    {
        public float m00;
        public float m01;
        public float m02;
        public float m10;
        public float m11;
        public float m12;
        public float m20;
        public float m21;
        public float m22;
    }

    [ElementBoundaryStruct("Matrix2x2")]
    public struct Matrix2x2
    {
        public float m00;
        public float m01;
        public float m10;
        public float m11;
    }

    [ElementBoundaryStruct("Bounds")]
    public struct Bounds
    {
        public float upper;
        public float lower;
    }
}