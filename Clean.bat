@echo off

rem ���ϐ��ݒ�
set ROOT_DIR=.\
set EXTERNAL_DIR=.\External\
set DATA_DIR=.\Data\

rem �t�H���_�폜
call :REMOVE_DIR %ROOT_DIR%.vs\
call :REMOVE_DIR %ROOT_DIR%bin\
call :REMOVE_DIR %ROOT_DIR%obj\
call :REMOVE_DIR %EXTERNAL_DIR%DirectXTex\Bin\
call :REMOVE_DIR %EXTERNAL_DIR%DirectXTex\Shaders\Compiled\
rem call :REMOVE_DIR %DATA_DIR%Shader\

rem �t�@�C���폜
rem del %CLIENT_INI_FILE%
rem del /s *.user
rem del /s imgui.ini

goto :FINISH

rem �t�H���_�폜�֐�
:REMOVE_DIR
if exist %1\ (
	rd /s /q %1
)
exit /b ERRORLEVEL

rem �I��
:FINISH
rem pause
