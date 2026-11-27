#include <iostream>
#include "Console.h"

int main()
{
    Console::ClearScreen();
    std::string message = "\nHello World!\n";
    Console::PrintDivider();
    std::cout << Console::Colorize(message, Console::ColorRole::Brand);
    //Console::PlaySoundCue(Console::SoundCue::Prompt);
    Console::PrintHeader("Account Info");
    Console::PrintSuccess("Registration Successful!");
    Console::PrintError("Invalid Info!");
    Console::PrintWarning("Your account is falling below threshold!");

    // Testing the Print table

    //Case 1: Populated table;

    std::vector<std::string> headers = { "Member", "Paid", "Net balance" };
    std::vector<std::vector<std::string>> data = {
        {"Germain", "120.00", "12"},
        {"Germain", "120.00", "12"},
        {"Germain", "120.00", "12"},
        {"Germain", "120.00", "12"}
    };

    Console::PrintTable(headers, data);

    // Case 2: Empty table

    Console::PrintTable({ "Incomes", "Expenses" }, {});


    Console::Pause();
}
