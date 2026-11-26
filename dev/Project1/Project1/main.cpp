#include <iostream>
#include "Console.h"

int main()
{
    std::string message = "Hello World!\n";
    std::cout << Console::Colorize(message, Console::ColorRole::Brand);
    Console::PlaySoundCue(Console::SoundCue::Prompt);
}
