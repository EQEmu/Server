using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using StreamParser.Common.Daybreak;
using System;

namespace StreamParser
{
    class Program
    {
        static void Main(string[] args)
        {
            CreateHostBuilder(args).Build().Run();
        }

        public static IHostBuilder CreateHostBuilder(string[] args)
        {
            return Host.CreateDefaultBuilder(args)
                .ConfigureServices((hostContext, services) =>
                {
                    services.AddScoped<IParser, Parser>();
                    services.AddHostedService<ConsoleHostedService>();
                });
        }
    }
}
