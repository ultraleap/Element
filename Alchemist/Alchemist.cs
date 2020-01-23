namespace Alchemist
{
    using System.Linq;
    using CommandLine;

    internal static class Alchemist
    {
        private static int Main(string[] args)
        {
            var commandTypes = typeof(Alchemist).Assembly.GetTypes()
                                                .Where(t => !t.IsAbstract && typeof(BaseCommand).IsAssignableFrom(t))
                                                .ToArray();

            return Parser.Default.ParseArguments(args, commandTypes)
                         .MapResult((BaseCommand cmd) => cmd.Invoke(), errors => 1);
        }
    }
}