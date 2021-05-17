namespace ResultNET
{
    public interface ITraceContext
    {
        ResultMessage? Trace(MessageInfo info, string? message);
    }

    public static class TraceContextExtensions
    {
        public static ResultMessage? Trace(this ITraceContext context,
            MessageLevel level,
            string? message,
            string? messageTypePrefix = null,
            string? messageName = null) =>
            context.Trace(new MessageInfo(messageTypePrefix ?? string.Empty, messageName, level, null), message);
    }
}