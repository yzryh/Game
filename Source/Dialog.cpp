#include "Dialog.h"

static char pathBuffer[MAX_PATH];

// [�t�@�C�����J��]�_�C�A���O�{�b�N�X��\��
DialogResult Dialog::OpenFileName(char* filepath, int size, const char* filter, const char* title, HWND hWnd, bool multiSelect)
{
	// �����p�X�ݒ�
	char dirname[MAX_PATH];
	if (filepath[0] != '0')
	{
		// �f�B���N�g���p�X�擾
		::_splitpath_s(filepath, nullptr, 0, dirname, MAX_PATH, nullptr, 0, nullptr, 0);
	}
	else
	{
		filepath[0] = dirname[0] = '\0';
	}
	if ((dirname[0] == '\0'))
	{
		strcpy_s(dirname, MAX_PATH, pathBuffer);
	}
	// lpstrInitialDir �� \ �łȂ��Ǝ󂯕t���Ȃ�
	for (char* p = dirname; *p != '\0'; p++)
	{
		if (*p == '/')
			* p = '\\';
	}

	if (filter == nullptr)
	{
		filter = "All Files\0*.*\0\0";
	}

	// �\���̃Z�b�g
	OPENFILENAMEA	ofn;
	memset(&ofn, 0, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = size;
	ofn.lpstrTitle = title;
	ofn.lpstrInitialDir = (dirname[0] != '\0') ? dirname : nullptr;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if (multiSelect)
	{
		ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	}

	// �J�����g�f�B���N�g���擾
	char currentDir[MAX_PATH];
	if (!::GetCurrentDirectoryA(MAX_PATH, currentDir))
	{
		currentDir[0] = '\0';
	}

	// �_�C�A���O�I�[�v��
	if (::GetOpenFileNameA(&ofn) == FALSE)
	{
		return DialogResult::Cancel;
	}

	// �J�����g�f�B���N�g�����A
	if (currentDir[0] != '\0')
	{
		::SetCurrentDirectoryA(currentDir);
	}

	// �ŏI�p�X���L��
	strcpy_s(pathBuffer, MAX_PATH, filepath);

	return DialogResult::OK;
}

// [�t�@�C����ۑ�]�_�C�A���O�{�b�N�X��\��
DialogResult Dialog::SaveFileName(char* filepath, int size, const char* filter, const char* title, const char* ext, HWND hWnd)
{
	// �����p�X�ݒ�
	char dirname[MAX_PATH];
	if (filepath[0] != '0')
	{
		// �f�B���N�g���p�X�擾
		::_splitpath_s(filepath, nullptr, 0, dirname, MAX_PATH, nullptr, 0, nullptr, 0);
	}
	else
	{
		filepath[0] = dirname[0] = '\0';
	}
	if ((dirname[0] == '\0'))
	{
		strcpy_s(dirname, MAX_PATH, pathBuffer);
	}
	// lpstrInitialDir �� \ �łȂ��Ǝ󂯕t���Ȃ�
	for (char* p = dirname; *p != '\0'; p++)
	{
		if (*p == '/')
			* p = '\\';
	}

	if (filter == nullptr)
	{
		filter = "All Files\0*.*\0\0";
	}

	// �\���̃Z�b�g
	OPENFILENAMEA	ofn;
	memset(&ofn, 0, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = size;
	ofn.lpstrTitle = title;
	ofn.lpstrInitialDir = (dirname[0] != '\0') ? dirname : nullptr;
	ofn.lpstrDefExt = ext;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

	// �J�����g�f�B���N�g���擾
	char current_dir[MAX_PATH];
	if (!::GetCurrentDirectoryA(MAX_PATH, current_dir))
	{
		current_dir[0] = '\0';
	}

	// �_�C�A���O�I�[�v��
	if (::GetSaveFileNameA(&ofn) == FALSE)
	{
		return DialogResult::Cancel;
	}

	// �J�����g�f�B���N�g�����A
	if (current_dir[0] != '\0')
	{
		::SetCurrentDirectoryA(current_dir);
	}

	// �ŏI�p�X���L��
	strcpy_s(pathBuffer, MAX_PATH, filepath);

	return DialogResult::OK;
}
