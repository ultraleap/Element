namespace Element
{
    public static class MessageExtensions
    {
        public static readonly string TypeString = "ELE";
        public static CompilerMessage? Trace(this Context context, EleMessageCode eleMessageCode, string? contextString) => context.Trace(TypeString, (int) eleMessageCode, contextString);
        public static void Append(this ResultBuilder builder, EleMessageCode eleMessageCode, string? context)
        {
            if (builder.Context.Trace(eleMessageCode, context) is {} msg) builder.Append(msg);
        }
        public static void Append<T>(this ResultBuilder<T> builder, EleMessageCode eleMessageCode, string? context)
        {
            if (builder.Context.Trace(eleMessageCode, context) is {} msg) builder.Append(msg);
        }
    }
    
    public enum EleMessageCode
    {
        // GENERAL PURPOSE
        Success = 0,
        FileAccessError = 25,
        ArgumentNotFound = 26,
        ArgumentOutOfRange = 28,
        InvalidCast = 30,
        UnknownError = 9999,
        
        // LOADING SOURCE/PACKAGES/PARSING
        ParseError= 9,
        DuplicateSourceFile = 27,
        PackageNotFound = 39,
        
        // VALIDATION
        MultipleDefinitions = 2,
        IntrinsicNotFound = 4,
        MissingPorts = 13,
        InvalidIdentifier = 15,
        StructCannotHaveReturnType = 19,
        IntrinsicCannotHaveBody = 20,
        MissingFunctionBody = 21,
        MultipleIntrinsicLocations = 29,
        FunctionMissingReturn = 31,
        PortListCannotContainDiscards = 32,
        PortListDeclaresDefaultArgumentBeforeNonDefault = 33,
        IntrinsicConstraintCannotSpecifyFunctionSignature = 34,
        
        // COMPILATION
        SerializationError = 1,
        InvalidCompileTarget = 3,
        LocalShadowing = 5,
        ArgumentCountMismatch = 6,
        IdentifierNotFound = 7,
        ConstraintNotSatisfied = 8,
        InvalidBoundaryFunction = 10,
        RecursionNotAllowed = 11,
        MissingBoundaryConverter = 12,
        TypeError = 14,
        InvalidExpression = 16,
        InvalidReturnType = 17,
        InvalidBoundaryData = 18,
        CannotBeUsedAsInstanceFunction = 22,
        FunctionCannotBeUncurried = 23,
        NotCompileConstant = 24,
        InfiniteLoop = 35,
        NotFunction = 36,
        NotIndexable = 37,
        NotConstraint = 38,
        CallStackLimitReached = 40,
    }
}