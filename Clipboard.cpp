#include "Clipboard.h"
#include <stdexcept>
#include <windows.h>
#include <wincrypt.h>
#include <vector>
#include <ctime>
#include <cstring>

// 클립보드 열기 (비밀번호 기반)
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

// 클립보드 열기 (토큰 기반)
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

// 클립보드 열기 (사용자 계정 기반)
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

// 클립보드 닫기
void Clipboard::Close()
{
    if (isOpen)
    {
        CloseClipboard();
        isOpen = false;
    }
}

// 텍스트 설정
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

// 현재 화면을 캡처하여 비트맵으로 반환
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

// 클립보드 포맷 확인
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

// 비밀번호 설정
void Clipboard::SetPassword(const std::string& newPassword)
{
    password = newPassword;
}

// 사용자 계정 추가
void Clipboard::AddUser(const std::string& username, const std::string& password, UserRole role)
{
    users.push_back({ username, password, role });
}

// 토큰 생성
std::string Clipboard::GenerateToken(const std::string& username)
{
    std::string token = username + "_token_" + std::to_string(std::time(nullptr));
    tokens[token] = std::time(nullptr) + 3600; // 토큰 유효기간: 1시간
    return token;
}

// 비밀번호 확인
bool Clipboard::VerifyPassword(const std::string& inputPassword) const
{
    return inputPassword == password;
}

// 사용자 인증
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

// 토큰 검증
bool Clipboard::ValidateToken(const std::string& token) const
{
    auto it = tokens.find(token);
    if (it != tokens.end() && std::time(nullptr) < it->second)
    {
        return true;
    }
    return false;
}

// 권한 확인
bool Clipboard::HasPermission(UserRole requiredRole) const
{
    return currentUserRole == requiredRole;
}
