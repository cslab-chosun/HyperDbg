/**
 * @file load.cpp
 * @author Sina Karvandi (sina@rayanfam.com)
 * @brief load command
 * @details
 * @version 0.1
 * @date 2020-05-27
 *
 * @copyright This project is released under the GNU Public License v3.
 *
 */
#include "pch.h"

//
// Global Variables
//
extern BOOLEAN g_IsConnectedToHyperDbgLocally;
extern BOOLEAN g_IsDebuggerModulesLoaded;

/**
 * @brief help of load command
 *
 * @return VOID
 */
VOID CommandLoadHelp() {
  ShowMessages("load : installs the drivers and load the modules.\n\n");
  ShowMessages("syntax : \tload [module name]\n");
  ShowMessages("\t\te.g : load vmm\n");
}

/**
 * @brief load vmm module
 *
 * @return BOOLEAN
 */
BOOLEAN CommandLoadVmmModule() {

  BOOL Status;
  HANDLE hToken;

  //
  // Enable Debug privilege
  //
  Status =
      OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
  if (!Status) {
    ShowMessages("OpenProcessToken failed, error : %u \n");
    return FALSE;
  }

  Status = SetPrivilege(hToken, SE_DEBUG_NAME, TRUE);
  if (!Status) {
    CloseHandle(hToken);
    return FALSE;
  }

  if (HyperdbgInstallVmmDriver()) {
    return FALSE;
  }

  if (HyperdbgLoadVmm()) {
    return FALSE;
  }

  //
  // If we reach here so the module are loaded
  //
  g_IsDebuggerModulesLoaded = TRUE;

  return TRUE;
}

/**
 * @brief load command handler
 *
 * @param SplittedCommand
 * @param Command
 * @return VOID
 */
VOID CommandLoad(vector<string> SplittedCommand, string Command) {

  if (SplittedCommand.size() != 2) {
    ShowMessages("incorrect use of 'load'\n\n");
    CommandLoadHelp();
    return;
  }

  if (!g_IsConnectedToHyperDbgLocally) {
    ShowMessages("You're not connected to any instance of HyperDbg, did you "
                 "use '.connect'? \n");
    return;
  }

  //
  // Check for the module
  //
  if (!SplittedCommand.at(1).compare("vmm")) {

    //
    // Load VMM Module
    //
    ShowMessages("try to install and load the VMM driver...\n");

    if (!CommandLoadVmmModule()) {
      ShowMessages("Failed to install or load the driver\n");
      return;
    }

  } else {

    //
    // Module not found
    //
    ShowMessages("module not found, currently, 'vmm' is the only available "
                 "module for HyperDbg.\n");
  }
}
