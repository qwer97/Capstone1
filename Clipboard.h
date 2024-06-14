#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <ctime>

enum class UserRole {
    Admin,
    User
};

struct UserAccount {
    std::string username;
    std::string password;
    UserRole role;
};

class Clipboard
{
public:
    // 클립보드 열기 및 닫기
    bool OpenWithPassword(const std::string& password);
    bool OpenWithToken(const std::string& token);
    bool OpenWithAccount(const std::string& username, const std::string& password);
    void Close();

    // 클립보드에 텍스트 설정 및 가져오기
    bool SetText(const std::wstring& text);
    std::wstring GetText();

    // 클립보드에 이미지 설정 및 가져오기
    bool SetImage(); // 캡처된 화면을 클립보드에 설정
    HBITMAP GetImage();
    HBITMAP CaptureScreen();

    // 클립보드 데이터 존재 여부 확인 (특정 format의)
    bool IsClipboardFormatAvailable(UINT format) const;

    // 비밀번호 설정
    void SetPassword(const std::string& newPassword);

    // 사용자 계정 관리
    void AddUser(const std::string& username, const std::string& password, UserRole role);

    // 토큰 관리
    std::string GenerateToken(const std::string& username);

private:
    bool isOpen = false;
    std::wstring clipboardText;
    HBITMAP clipboardImage = nullptr;
    std::string password;
    std::vector<UserAccount> users; // 유저 저장용
    std::map<std::string, std::time_t> tokens;
    UserRole currentUserRole;

    bool VerifyPassword(const std::string& inputPassword) const;
    bool AuthenticateUser(const std::string& username, const std::string& password);
    bool ValidateToken(const std::string& token) const;
    bool HasPermission(UserRole requiredRole) const;
};
