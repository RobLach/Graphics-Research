using System;

namespace IrvingParkRoad
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        static void Main(string[] args)
        {
            using (IPR game = new IPR())
            {
                game.Run();
            }
        }
    }
}

