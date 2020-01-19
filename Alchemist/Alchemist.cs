namespace Alchemist
{
    using System;
    using System.Linq;
    using CommandLine;

    internal static class Alchemist
    {
        public static void Log(string message) => Console.WriteLine(message);

        public static void LogError(string message) => Console.Error.WriteLine(message);

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