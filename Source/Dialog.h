#pragma once

#include <Windows.h>

// �_�C�A���O���U���g
enum class DialogResult
{
	Yes,
	No,
	OK,
	Cancel
};

// �_�C�A���O
class Dialog
{
public:
	// [�t�@�C�����J��]�_�C�A���O�{�b�N�X��\��
	static DialogResult OpenFileName(char* filepath, int size, const char* filter = nullptr, const char* title = nullptr, HWND hWnd = NULL, bool multiSelect = false);

	// [�t�@�C����ۑ�]�_�C�A���O�{�b�N�X��\��
	static DialogResult SaveFileName(char* filepath, int size, const char* filter = nullptr, const char* title = nullptr, const char* ext = nullptr, HWND hWnd = NULL);
};
