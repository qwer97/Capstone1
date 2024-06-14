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
    // Ŭ������ ���� �� �ݱ�
    bool OpenWithPassword(const std::string& password);
    bool OpenWithToken(const std::string& token);
    bool OpenWithAccount(const std::string& username, const std::string& password);
    void Close();

    // Ŭ�����忡 �ؽ�Ʈ ���� �� ��������
    bool SetText(const std::wstring& text);
    std::wstring GetText();

    // Ŭ�����忡 �̹��� ���� �� ��������
    bool SetImage(); // ĸó�� ȭ���� Ŭ�����忡 ����
    HBITMAP GetImage();
    HBITMAP CaptureScreen();

    // Ŭ������ ������ ���� ���� Ȯ�� (Ư�� format��)
    bool IsClipboardFormatAvailable(UINT format) const;

    // ��й�ȣ ����
    void SetPassword(const std::string& newPassword);

    // ����� ���� ����
    void AddUser(const std::string& username, const std::string& password, UserRole role);

    // ��ū ����
    std::string GenerateToken(const std::string& username);

private:
    bool isOpen = false;
    std::wstring clipboardText;
    HBITMAP clipboardImage = nullptr;
    std::string password;
    std::vector<UserAccount> users; // ���� �����
    std::map<std::string, std::time_t> tokens;
    UserRole currentUserRole;

    bool VerifyPassword(const std::string& inputPassword) const;
    bool AuthenticateUser(const std::string& username, const std::string& password);
    bool ValidateToken(const std::string& token) const;
    bool HasPermission(UserRole requiredRole) const;
};
