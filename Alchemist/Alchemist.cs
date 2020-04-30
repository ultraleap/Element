using System.Linq;
using CommandLine;

namespace Alchemist
{
    internal static class Alchemist
    {
        private static int Main(string[] args) =>
            Parser.Default.ParseArguments(args, typeof(Alchemist).Assembly.GetTypes()
                    .Where(t => !t.IsAbstract && typeof(BaseCommand).IsAssignableFrom(t))
                    .ToArray())
                .MapResult((BaseCommand cmd) => cmd.Invoke(string.Join(' ', args)),
                    errors => 1);
    }
}