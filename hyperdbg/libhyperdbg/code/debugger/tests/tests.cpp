/**
 * @file tests.cpp
 * @author Sina Karvandi (sina@hyperdbg.org)
 * @brief perform tests
 * @details
 * @version 0.1
 * @date 2020-09-14
 *
 * @copyright This project is released under the GNU Public License v3.
 *
 */
#include "pch.h"

//
// Global Variables
//
extern TCHAR g_TestLocation[MAX_PATH];

/**
 * @brief
 *
 * @param BufferLength Setup test process name
 * @return BOOLEAN
 */
BOOLEAN
SetupTestName(_Inout_updates_bytes_all_(BufferLength) PCHAR TestLocation,
              ULONG                                         BufferLength)
{
    HANDLE  fileHandle;
    DWORD   driverLocLen = 0;
    HMODULE ProcHandle   = GetModuleHandle(NULL);
    char *  Pos;

    //
    // Get the current directory.
    //

    //
    // We use the location of running exe instead of
    // finding driver based on current directory
    //

    /*
  driverLocLen = GetCurrentDirectory(BufferLength, DriverLocation);

  if (driverLocLen == 0) {

    ShowMessages("err, GetCurrentDirectory failed (%x)\n", GetLastError());

    return FALSE;
  }
  */

    GetModuleFileName(ProcHandle, TestLocation, BufferLength);

    Pos = strrchr(TestLocation, '\\');
    if (Pos != NULL)
    {
        //
        // this will put the null terminator here. you can also copy to
        // another string if you want, we can also use PathCchRemoveFileSpec
        //
        *Pos = '\0';
    }

    //
    // Setup path name to driver file.
    //
    if (FAILED(StringCbCat(TestLocation, BufferLength, "\\" TEST_PROCESS_NAME)))
    {
        return FALSE;
    }

    //
    // Insure test file is in the specified directory
    //
    if ((fileHandle = CreateFile(TestLocation, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) ==
        INVALID_HANDLE_VALUE)
    {
        ShowMessages("%s.exe is not loaded\n", TEST_PROCESS_NAME);

        //
        // Indicate failure.
        //
        return FALSE;
    }

    //
    // Close open file handle.
    //
    if (fileHandle)
    {
        CloseHandle(fileHandle);
    }

    //
    // Indicate success.
    //
    return TRUE;
}

/**
 * @brief Create a Process And Open Pipe Connection object
 *
 * @param ConnectionPipeHandle Pointer to receive Pipe Handle
 * @param ThreadHandle Pointer to receive Thread Handle
 * @param ProcessHandle Pointer to receive Process Handle
 * @return BOOLEAN
 */
BOOLEAN
CreateProcessAndOpenPipeConnection(PHANDLE ConnectionPipeHandle,
                                   PHANDLE ThreadHandle,
                                   PHANDLE ProcessHandle)
{
    HANDLE              PipeHandle;
    BOOLEAN             SentMessageResult;
    UINT32              ReadBytes;
    char *              BufferToRead;
    char *              BufferToSend;
    char                HandshakeBuffer[] = "Hello, Dear Test Process... Yes, I'm HyperDbg Debugger :)";
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO         StartupInfo;
    char                CmdArgs[] = TEST_PROCESS_NAME " im-hyperdbg";

    PipeHandle = NamedPipeServerCreatePipe("\\\\.\\Pipe\\HyperDbgTests",
                                           TEST_CASE_MAXIMUM_BUFFERS_TO_COMMUNICATE,
                                           TEST_CASE_MAXIMUM_BUFFERS_TO_COMMUNICATE);
    if (!PipeHandle)
    {
        //
        // Error in creating handle
        //
        return FALSE;
    }

    BufferToRead = (char *)malloc(TEST_CASE_MAXIMUM_BUFFERS_TO_COMMUNICATE);

    if (!BufferToRead)
    {
        //
        // Enable to allocate buffer
        //
        return FALSE;
    }

    BufferToSend = (char *)malloc(TEST_CASE_MAXIMUM_BUFFERS_TO_COMMUNICATE);

    if (!BufferToSend)
    {
        //
        // Enable to allocate buffer
        //
        free(BufferToSend);
        return FALSE;
    }

    RtlZeroMemory(BufferToRead, TEST_CASE_MAXIMUM_BUFFERS_TO_COMMUNICATE);
    RtlZeroMemory(BufferToSend, TEST_CASE_MAXIMUM_BUFFERS_TO_COMMUNICATE);

    strcpy(BufferToSend, HandshakeBuffer);

    //
    // Create the Test Process
    //

    ZeroMemory(&StartupInfo, sizeof(StartupInfo));

    //
    // the only compulsory field
    //
    StartupInfo.cb = sizeof StartupInfo;

    //
    // Set-up path
    //
    if (!SetupTestName(g_TestLocation, sizeof(g_TestLocation)))
    {
        //
        // Test process not found
        //
        free(BufferToRead);
        free(BufferToSend);

        return FALSE;
    }

    if (CreateProcess(g_TestLocation, CmdArgs, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo))
    {
        //
        // Wait for message from the target process
        //
        if (!NamedPipeServerWaitForClientConntection(PipeHandle))
        {
            //
            // Error in connection
            //
            free(BufferToRead);
            free(BufferToSend);

            return FALSE;
        }

        ReadBytes =
            NamedPipeServerReadClientMessage(PipeHandle, BufferToRead, TEST_CASE_MAXIMUM_BUFFERS_TO_COMMUNICATE);

        if (!ReadBytes)
        {
            //
            // Nothing to read
            //

            free(BufferToRead);
            free(BufferToSend);

            return FALSE;
        }

        if (strcmp(BufferToRead, "Hey there, Are you HyperDbg?") == 0)
        {
            //
            // *** Connected to the remote process ***
            //

            //
            // Send the hand shake message
            //
            SentMessageResult = NamedPipeServerSendMessageToClient(
                PipeHandle,
                BufferToSend,
                (UINT32)strlen(BufferToSend) + 1);

            if (!SentMessageResult)
            {
                //
                // error in sending
                //

                free(BufferToRead);
                free(BufferToSend);

                return FALSE;
            }

            //
            // Receive the request for the test case number
            //
            RtlZeroMemory(BufferToRead, TEST_CASE_MAXIMUM_BUFFERS_TO_COMMUNICATE);

            ReadBytes = NamedPipeServerReadClientMessage(PipeHandle, BufferToRead, TEST_CASE_MAXIMUM_BUFFERS_TO_COMMUNICATE);

            if (!ReadBytes)
            {
                //
                // Nothing to read
                //

                free(BufferToRead);
                free(BufferToSend);

                return FALSE;
            }

            if (strcmp(BufferToRead, "Wow! I miss you... Would you plz send test cases?") == 0)
            {
                //
                // Set the output handles
                //
                *ConnectionPipeHandle = PipeHandle;
                *ThreadHandle         = ProcessInfo.hThread;
                *ProcessHandle        = ProcessInfo.hProcess;

                free(BufferToRead);
                free(BufferToSend);

                return TRUE;
            }

            ShowMessages("err, could not handshake with the test process\n");

            free(BufferToRead);
            free(BufferToSend);

            return FALSE;
        }
        else
        {
            ShowMessages("err, the process could not be started\n");

            free(BufferToRead);
            free(BufferToSend);

            return FALSE;
        }
    }

    free(BufferToRead);
    free(BufferToSend);

    return FALSE;
}

/**
 * @brief Opens test process
 *
 * @param ThreadHandle Pointer to receive Thread Handle
 * @param ProcessHandle Pointer to receive Process Handle
 * @param Args Arguments
 *
 * @return BOOLEAN
 */
BOOLEAN
OpenHyperDbgTestProcess(PHANDLE ThreadHandle,
                        PHANDLE ProcessHandle,
                        CHAR *  Args)
{
    PROCESS_INFORMATION ProcessInfo       = {0};
    STARTUPINFO         StartupInfo       = {0};
    CHAR                CmdArgs[MAX_PATH] = {0};

    //
    // the only compulsory field
    //
    StartupInfo.cb = sizeof(StartupInfo);

    //
    // Set-up path
    //
    if (!SetupTestName(g_TestLocation, sizeof(g_TestLocation)))
    {
        return FALSE;
    }

    //
    // Format CmdArgs to include executable and its arguments
    //
    sprintf_s(CmdArgs, sizeof(CmdArgs), "\"%s\" %s", g_TestLocation, Args);

    //
    // Open test process
    //
    if (CreateProcessA(NULL, CmdArgs, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo))
    {
        //
        // Set the output handles
        //
        *ThreadHandle  = ProcessInfo.hThread;
        *ProcessHandle = ProcessInfo.hProcess;

        return TRUE;
    }
    else
    {
        ShowMessages("err, CreateProcess failed with error %x\n", GetLastError());
    }

    return FALSE;
}

/**
 * @brief Close the pipe connection and the remote process
 *
 * @param ConnectionPipeHandle Handle of remote pipe connection
 * @param ThreadHandle Handle of test thread
 * @param ProcessHandle Handle of test process
 * @return VOID
 */
VOID
CloseProcessAndClosePipeConnection(HANDLE ConnectionPipeHandle,
                                   HANDLE ThreadHandle,
                                   HANDLE ProcessHandle)
{
    //
    // Close the connection and handles
    //
    NamedPipeServerCloseHandle(ConnectionPipeHandle);
    CloseHandle(ThreadHandle);
    CloseHandle(ProcessHandle);
}
