#include <iostream>
#include <string>
#include "Clipboard.h"

void ShowMenu() {
    std::cout << "1. Set Text to Clipboard\n";
    std::cout << "2. Get Text from Clipboard\n";
    std::cout << "3. Set Image to Clipboard\n";
    std::cout << "4. Get Image from Clipboard\n";
    std::cout << "5. Generate Token\n";
    std::cout << "6. Exit\n";
    std::cout << "Select an option: ";
}

int main() {
    Clipboard clipboard;
    clipboard.SetPassword("p123");
    clipboard.AddUser("u1", "p1", UserRole::Admin);
    clipboard.AddUser("u2", "p2", UserRole::User);

    std::string token = clipboard.GenerateToken("u1");

    std::string inputPassword;
    std::string inputToken;
    std::string inputUsername;
    std::string inputUserPassword;

    bool authenticated = false;
    char choice;

    while (!authenticated) {
        std::cout << "Choose authentication method:\n";
        std::cout << "1. Password\n";
        std::cout << "2. Token\n";
        std::cout << "3. User Account\n";
        std::cout << "Select an option: ";
        std::cin >> choice;

        try {
            switch (choice) {
            case '1':
                std::cout << "Enter password: ";
                std::cin >> inputPassword;
                authenticated = clipboard.OpenWithPassword(inputPassword);
                break;
            case '2':
                std::cout << "Enter token: ";
                std::cin >> inputToken;
                authenticated = clipboard.OpenWithToken(inputToken);
                break;
            case '3':
                std::cout << "Enter username: ";
                std::cin >> inputUsername;
                std::cout << "Enter password: ";
                std::cin >> inputUserPassword;
                authenticated = clipboard.OpenWithAccount(inputUsername, inputUserPassword);
                break;
            default:
                std::cout << "Invalid option. Try again.\n";
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }
    }

    while (true) {
        ShowMenu();
        std::cin >> choice;

        try {
            switch (choice) {
            case '1': {
                std::wstring text;
                std::wcout << L"Enter text to set to clipboard: ";
                std::wcin.ignore();
                std::getline(std::wcin, text);
                clipboard.SetText(text);
                std::wcout << L"Text set to clipboard.\n";
                break;
            }
            case '2': {
                std::wstring text = clipboard.GetText();
                std::wcout << L"Clipboard Text: " << text << std::endl;
                break;
            }
            case '3': {
                clipboard.SetImage();
                std::cout << "Image set to clipboard.\n";
                break;
            }
            case '4': {
                HBITMAP hBitmap = clipboard.GetImage();
                if (hBitmap) {
                    std::cout << "Image retrieved from clipboard.\n";
                }
                else {
                    std::cout << "No image in clipboard.\n";
                }
                break;
            }
            case '5': {
                std::string username;
                std::cout << "Enter username to generate token: ";
                std::cin >> username;
                std::string newToken = clipboard.GenerateToken(username);
                std::cout << "Generated Token: " << newToken << std::endl;
                break;
            }
            case '6':
                clipboard.Close();
                std::cout << "Exiting program.\n";
                return 0;
            default:
                std::cout << "Invalid option. Try again.\n";
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }
    }

    return 0;
}