#include "Clipboard.h"
#include <stdexcept>
#include <windows.h>
#include <wincrypt.h>
#include <vector>
#include <ctime>
#include <cstring>

// Ŭ������ ���� (��й�ȣ ���)
bool Clipboard::OpenWithPassword(const std::string& inputPassword)
{
    if (!VerifyPassword(inputPassword))
    {
        throw std::runtime_error("Incorrect password.");
    }
    if (isOpen)
    {
        throw std::runtime_error("Clipboard is already open.");
    }
    isOpen = true;
    currentUserRole = UserRole::Admin; 
    return true;
}

// Ŭ������ ���� (��ū ���)
bool Clipboard::OpenWithToken(const std::string& token)
{
    if (!ValidateToken(token))
    {
        throw std::runtime_error("Invalid or expired token.");
    }
    if (isOpen)
    {
        throw std::runtime_error("Clipboard is already open.");
    }
    isOpen = true;
    
    for (const auto& user : users) {
        if (token.find(user.username) != std::string::npos) {
            currentUserRole = user.role;
            break;
        }
    }
    return true;
}

// Ŭ������ ���� (����� ���� ���)
bool Clipboard::OpenWithAccount(const std::string& username, const std::string& password)
{
    if (!AuthenticateUser(username, password))
    {
        throw std::runtime_error("Invalid username or password.");
    }
    if (isOpen)
    {
        throw std::runtime_error("Clipboard is already open.");
    }
    isOpen = true;
    for (const auto& user : users) {
        if (user.username == username) {
            currentUserRole = user.role;
            break;
        }
    }
    return true;
}

// Ŭ������ �ݱ�
void Clipboard::Close()
{
    if (isOpen)
    {
        CloseClipboard();
        isOpen = false;
    }
}

// �ؽ�Ʈ ����
bool Clipboard::SetText(const std::wstring& text)
{
    if (!isOpen)
    {
        throw std::runtime_error("Clipboard is not open.");
    }
    if (!HasPermission(UserRole::Admin))
    {
        throw std::runtime_error("Permission denied.");
    }

    if (!OpenClipboard(nullptr))
    {
        throw std::runtime_error("Failed to open clipboard.");
    }

    if (!EmptyClipboard())
    {
        CloseClipboard();
        throw std::runtime_error("Failed to empty clipboard.");
    }

    size_t dataSize = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dataSize);
    if (!hGlobal)
    {
        CloseClipboard();
        throw std::runtime_error("Failed to allocate global memory.");
    }

    memcpy(GlobalLock(hGlobal), text.c_str(), dataSize);
    GlobalUnlock(hGlobal);

    if (!SetClipboardData(CF_UNICODETEXT, hGlobal))
    {
        GlobalFree(hGlobal);
        CloseClipboard();
        throw std::runtime_error("Failed to set clipboard data.");
    }

    CloseClipboard();
    return true;
}

std::wstring Clipboard::GetText()
{
    if (!isOpen)
    {
        throw std::runtime_error("Clipboard is not open.");
    }
    if (!HasPermission(UserRole::Admin))
    {
        throw std::runtime_error("Permission denied.");
    }

    if (!OpenClipboard(nullptr))
    {
        throw std::runtime_error("Failed to open clipboard.");
    }

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);

    if (hData == nullptr)
    {
        CloseClipboard();
        throw std::runtime_error("Failed to get clipboard data.");
    }

    wchar_t* pData = static_cast<wchar_t*>(GlobalLock(hData));
    if (pData == nullptr)
    {
        CloseClipboard();
        throw std::runtime_error("Failed to lock global memory.");
    }

    std::wstring text(pData);
    GlobalUnlock(hData);

    CloseClipboard();
    return text;
}

// ���� ȭ���� ĸó�Ͽ� ��Ʈ������ ��ȯ
HBITMAP Clipboard::CaptureScreen()
{
    HDC hScreen = GetDC(nullptr);
    HDC hDC = CreateCompatibleDC(hScreen);
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
    HGDIOBJ oldObj = SelectObject(hDC, hBitmap);
    BitBlt(hDC, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);
    SelectObject(hDC, oldObj);
    DeleteDC(hDC);
    ReleaseDC(nullptr, hScreen);
    return hBitmap;
}

bool Clipboard::SetImage()
{
    HBITMAP hBitmap = CaptureScreen();

    if (!isOpen)
    {
        throw std::runtime_error("Clipboard is not open.");
    }
    if (!HasPermission(UserRole::Admin))
    {
        throw std::runtime_error("Permission denied.");
    }

    if (!OpenClipboard(nullptr))
    {
        throw std::runtime_error("Failed to open clipboard.");
    }

    if (!EmptyClipboard())
    {
        CloseClipboard();
        throw std::runtime_error("Failed to empty clipboard.");
    }

    if (!SetClipboardData(CF_BITMAP, hBitmap))
    {
        CloseClipboard();
        throw std::runtime_error("Failed to set clipboard data.");
    }

    CloseClipboard();
    return true;
}

HBITMAP Clipboard::GetImage()
{
    if (!isOpen)
    {
        throw std::runtime_error("Clipboard is not open.");
    }
    if (!HasPermission(UserRole::Admin))
    {
        throw std::runtime_error("Permission denied.");
    }

    if (!OpenClipboard(nullptr))
    {
        throw std::runtime_error("Failed to open clipboard.");
    }

    HBITMAP hBitmap = static_cast<HBITMAP>(GetClipboardData(CF_BITMAP));
    if (!hBitmap)
    {
        CloseClipboard();
        throw std::runtime_error("Failed to get clipboard data.");
    }

    CloseClipboard();
    return hBitmap;
}

// Ŭ������ ���� Ȯ��
bool Clipboard::IsClipboardFormatAvailable(UINT format) const
{
    if (!isOpen)
    {
        throw std::runtime_error("Clipboard is not open.");
    }
    if (!HasPermission(UserRole::Admin))
    {
        throw std::runtime_error("Permission denied.");
    }

    return ::IsClipboardFormatAvailable(format) != 0;
}

// ��й�ȣ ����
void Clipboard::SetPassword(const std::string& newPassword)
{
    password = newPassword;
}

// ����� ���� �߰�
void Clipboard::AddUser(const std::string& username, const std::string& password, UserRole role)
{
    users.push_back({ username, password, role });
}

// ��ū ����
std::string Clipboard::GenerateToken(const std::string& username)
{
    std::string token = username + "_token_" + std::to_string(std::time(nullptr));
    tokens[token] = std::time(nullptr) + 3600; // ��ū ��ȿ�Ⱓ: 1�ð�
    return token;
}

// ��й�ȣ Ȯ��
bool Clipboard::VerifyPassword(const std::string& inputPassword) const
{
    return inputPassword == password;
}

// ����� ����
bool Clipboard::AuthenticateUser(const std::string& username, const std::string& password)
{
    for (const auto& user : users)
    {
        if (user.username == username && user.password == password)
        {
            return true;
        }
    }
    return false;
}

// ��ū ����
bool Clipboard::ValidateToken(const std::string& token) const
{
    auto it = tokens.find(token);
    if (it != tokens.end() && std::time(nullptr) < it->second)
    {
        return true;
    }
    return false;
}

// ���� Ȯ��
bool Clipboard::HasPermission(UserRole requiredRole) const
{
    return currentUserRole == requiredRole;
}
