///////////////////////////////////////////////////////////////////////////////
//
// �ļ�����
//     Injector.h
//     ����ע��Զ�̽�����ص�һ����, �����ռ�Ϊ Injector
//     ֧�� Windows9x, WindowsME, WindowsNT, Windows2000, WindowsXP
//
// ��Ȩ����
//     Copyright (c) 2009 ����Χ All Rights Reserved.
//
// ���¼�¼
//
//     2009��02��08�� : ����
//
///////////////////////////////////////////////////////////////////////////////
#ifndef INJECTOR_H
#define INJECTOR_H

#pragma warning(disable:4996)
#include <stddef.h>
#include <windows.h>
#include <TLHELP32.H>

namespace Injector
{
    //
    // ����ϵͳ�汾, ��װ�ɽṹ��
    //
    struct OS_VER
    {
        bool IsWin98;
        bool IsWinMe;
        bool IsWinXp;
    };


    //
    // ��ע����ص�һ�����ϵͳ API, ��װ�ɽṹ��
    //
    struct OS_API
    {
        HANDLE  ( WINAPI *CreateToolhelp32Snapshot )( DWORD  dwFlags, DWORD th32ProcessID );
        BOOL    ( WINAPI *Process32First )          ( HANDLE hSnapshot, LPPROCESSENTRY32 lppe ); 
        BOOL    ( WINAPI *Process32Next )           ( HANDLE hSnapshot, LPPROCESSENTRY32 lppe );
        BOOL    ( WINAPI *Module32First )           ( HANDLE hSnapshot, LPMODULEENTRY32 lpme ); 
        BOOL    ( WINAPI *Module32Next )            ( HANDLE hSnapshot, LPMODULEENTRY32 lpme );
        BOOL    ( WINAPI *Thread32First )           ( HANDLE hSnapshot, LPTHREADENTRY32 lpte );
        BOOL    ( WINAPI *Thread32Next )            ( HANDLE hSnapshot, LPTHREADENTRY32 lpte );
        HANDLE  ( WINAPI *OpenProcess )             ( DWORD dwDesiredAccess,
                                                      BOOL  bInheritHandle,
                                                      DWORD dwProcessId
                                                    );
        HANDLE  ( WINAPI *OpenThread )              ( DWORD dwDesiredAccess,
                                                      BOOL  bInheritHandle,
                                                      DWORD dwThreadId
                                                    );
        LPVOID  ( WINAPI *VirtualAllocEx )          ( HANDLE hProcess,
                                                      LPVOID lpAddress,
                                                      SIZE_T dwSize,
                                                      DWORD  flAllocationType,
                                                      DWORD  flProtect
                                                    );
        BOOL    ( WINAPI *VirtualFreeEx )           ( HANDLE hProcess,
                                                      LPVOID lpAddress,
                                                      SIZE_T dwSize,
                                                      DWORD  dwFreeType
                                                    );        
    };

    // 
    // ����
    //     �õ�����ϵͳ�汾
    //  
    // ����
    //
    // osVer
    //     �������ϵͳ�汾�ṹ�������ָ��
    //
    // ����ֵ
    //     false : ʧ��
    //     true  : �ɹ�
    //
    bool GetOsVer( OS_VER *osVer );

    // 
    // ����
    //     ���ݲ���ϵͳ�汾, ��̬�õ���ע�������ص�һ��ӿ�ͳһ��ϵͳ API
    //     ����ò���ϵͳ���ṩ����ĳ������, ����ɿ⸺��ģ��һ��
    //  
    // ����
    //
    // osApi
    //     ���ϵͳ API �ṹ�������ָ��
    //
    // ����ֵ
    //     false : ʧ��
    //     true  : �ɹ�
    //
    bool GetOsApi( OS_API *osApi );

    // 
    // ����
    //     ��ȡ����Ŀ����̵��������
    //  
    // ����
    //
    // exeName
    //     Ŀ����̵Ŀ�ִ���ļ���(������·��)
    // 
    // processInfo
    //     ����ý��� PROCESS_INFORMATION �ṹ�������ָ��
    //
    // ����ֵ
    //     false : ʧ��
    //     true  : �ɹ�
    //
    bool GetProcessInfo( const char *exeName, PROCESS_INFORMATION *processInfo );

    // 
    // ����
    //     ��ȡĿ�������һ��ģ����������
    //  
    // ����
    //
    // exeName
    //     Ŀ����̵Ŀ�ִ���ļ���(������·��)
    //
    // moduleName
    //     Ŀ�������һ��ģ����ļ���(������·��)
    //
    // moduleEntry32
    //     �����ģ�� MODULEENTRY32 �ṹ�������ָ��
    //
    // ����ֵ
    //     false : ʧ��
    //     true  : �ɹ�
    //
    bool GetModuleInfo( const char *exeName, const char *moduleName, MODULEENTRY32 *moduleEntry32 );

    // 
    // ����
    //     ��ģ��ע�뵽Ŀ�����
    // 
    // ����
    //
    // exeName
    //     Ŀ����̵Ŀ�ִ���ļ���(������·��)
    // 
    // modulePath
    //     ��ע�뵽Ŀ����̵�ģ��ľ���·���������Ŀ����̵����·��
    //
    // ����ֵ
    //     false : ʧ��
    //     true  : �ɹ�
    //
    bool InjectModule( const char *exeName, const char *modulePath );
}

#endif