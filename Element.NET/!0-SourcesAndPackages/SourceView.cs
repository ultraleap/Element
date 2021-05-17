using System;
using System.Collections.Generic;
using System.Linq;
using Element.AST;
using ResultNET;

namespace Element
{
    /// <summary>
    /// Represents a view over a SourceContext which retrieves values that match a given predicate.
    /// </summary>
    public class SourceView
    {
        public SourceContext SourceContext { get; }

        // The same cache is used for retrieving functions by identifier and full path because the strings cannot conflict
        private readonly Dictionary<string, Result<ValueWithLocation>> _cache;
        private readonly List<ValueWithLocation> _predicatedFunctions;
        private readonly Predicate<ValueWithLocation> _lookupPredicate;

        private SourceView(SourceContext sourceContext,
            Predicate<ValueWithLocation> lookupPredicate,
            List<ValueWithLocation> predicatedFunctions)
        {
            SourceContext = sourceContext;
            _cache = new Dictionary<string, Result<ValueWithLocation>>();
            _lookupPredicate = lookupPredicate;
            _predicatedFunctions = predicatedFunctions;
            
            // Check for duplicate functions using a hash set of their identifier
            var idSet = new HashSet<Identifier>();
            var context = Context.CreateFromSourceContext(sourceContext);
            foreach (var v in predicatedFunctions.Where(v => !idSet.Add(v.Identifier)))
            {
                // If there's a duplicate add an error to the cache which will be retrieved instead of the ambiguous match if a user attempts to get it by identifier
                _cache[v.Identifier.String] = context.Trace(MessageLevel.Error, $"Ambiguous function identifier '{v.Identifier}' - use full location to disambiguate");
            }
        }

        public static Result<SourceView> Create(SourceContext srcContext, Predicate<ValueWithLocation> lookupPredicate)
        {
            var context = Context.CreateFromSourceContext(srcContext);
            return srcContext.GlobalScope.EnumerateValues(context, resolvedValueFilter: lookupPredicate)
                             .Bind(matchingFunctions => matchingFunctions.Any()
                                  ? new Result<SourceView>(new SourceView(srcContext, lookupPredicate, matchingFunctions))
                                  : context.Trace(MessageLevel.Error, "No functions found matching the lookup predicate"));
        }

        private Result<ValueWithLocation> LookupFunction(string lookup, Func<ValueWithLocation, bool> lookupPredicate)
        {
            if (_cache.TryGetValue(lookup, out var cachedFunction)) { return cachedFunction; }
            
            var context = Context.CreateFromSourceContext(SourceContext);
            var found = _predicatedFunctions.FirstOrDefault(lookupPredicate);
            if (found == null) return _cache[lookup] = context.Trace(EleMessageCode.IdentifierNotFound, $"No function '{lookup}' was found");
            if (!_lookupPredicate(found)) return context.Trace(MessageLevel.Error, $"Value <{found.WrappedValue}> does not match the lookup predicate");
            
            return _cache[lookup] = found;
        }
        
        public Result<ValueWithLocation> GetFunctionByFullPath(string fullPath) => LookupFunction(fullPath, v => v.FullPath == fullPath);
        public Result<ValueWithLocation> GetFunctionByIdentifier(string identifier) => LookupFunction(identifier, v => v.Identifier.String == identifier);
        public IReadOnlyList<ValueWithLocation> AllFunctions => _predicatedFunctions;
    }
}