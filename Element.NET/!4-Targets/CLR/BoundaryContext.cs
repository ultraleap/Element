using System;
using System.Collections.Generic;
using Element.AST;
using Expression = System.Linq.Expressions.Expression;

namespace Element.CLR
{
    // TODO: Make IContext and pass through to a wrapped context
    public class BoundaryContext : Context
    {
        public BoundaryMap BoundaryMap { get; }
        public static BoundaryContext FromContext(Context context, BoundaryMap cache) => new BoundaryContext(context, cache);

        private BoundaryContext(Context context, BoundaryMap cache) : base(context.RootScope, context.CompilerOptions, context.StructuralTuples) => BoundaryMap = cache;

        public Result<IValue> LinqToElement(Expression parameter) => BoundaryMap.LinqToElement(parameter, this);
        public Result<Expression> ElementToLinq(IValue value, Type outputType, ConvertFunction convertFunction) => BoundaryMap.ElementToLinq(value, outputType, convertFunction, this);
        public Result<Type> ElementToClr(Struct elementStruct) => BoundaryMap.ElementToClr(elementStruct, this);
        public Result<Type> ElementToClr(IIntrinsicStructImplementation elementStruct) => BoundaryMap.ElementToClr(elementStruct, this);
        public Result SerializeClrInstance(object clrInstance, ICollection<float> floats) =>
            BoundaryMap.GetConverter(clrInstance.GetType(), this)
                       .Bind(converter => converter.SerializeClrInstance(clrInstance, floats, this));
    }
    
    public static class BoundaryContextExtensions
    {
        public static BoundaryContext ToBoundaryContext(this Context context, BoundaryMap cache) => BoundaryContext.FromContext(context, cache);
        public static Result<BoundaryContext> ToDefaultBoundaryContext(this Context context) => context.ToBoundaryContext(BoundaryMap.CreateDefault());
    }
}