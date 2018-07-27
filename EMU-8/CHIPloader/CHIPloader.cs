using System.IO;
using System;

class CHIPloader
{
	static void Main(){
		Console.WriteLine("Enter game name (all caps)\nEnsure game is located in directory with EMU-8.exe\nEnsure game has .bin file extension");
		string game = Console.ReadLine();
		using (StreamWriter writer = new StreamWriter("temp.txt")){
			writer.WriteLine(game + ".bin");
		}
	}
}