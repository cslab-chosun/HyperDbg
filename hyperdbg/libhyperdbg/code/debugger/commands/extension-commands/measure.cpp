/**
 * @file measure.cpp
 * @author Sina Karvandi (sina@hyperdbg.org)
 * @brief !measure command
 * @details
 * @version 0.1
 * @date 2020-08-06
 *
 * @copyright This project is released under the GNU Public License v3.
 *
 */
#include "pch.h"

//
// Global Variables
//
extern UINT64 g_CpuidAverage;
extern UINT64 g_CpuidStandardDeviation;
extern UINT64 g_CpuidMedian;

extern UINT64 g_RdtscAverage;
extern UINT64 g_RdtscStandardDeviation;
extern UINT64 g_RdtscMedian;

extern BOOLEAN g_TransparentResultsMeasured;

/**
 * @brief help of the !measure command
 *
 * @return VOID
 */
VOID
CommandMeasureHelp()
{
    ShowMessages(
        "This command is deprecated, you should not use it anymore!\n\n");
    ShowMessages(
        "!measure : measures the arguments needs for the '!hide' command.\n\n");

    ShowMessages("syntax : \t!measure [default]\n");

    ShowMessages("\n");
    ShowMessages("\t\te.g : !measure\n");
    ShowMessages("\t\te.g : !measure default\n");
}

/**
 * @brief !measure command handler
 *
 * @param CommandTokens
 * @param Command
 *
 * @return VOID
 */
VOID
CommandMeasure(vector<CommandToken> CommandTokens, string Command)
{
    BOOLEAN DefaultMode = FALSE;

    ShowMessages("This command is deprecated, you should not use it anymore!\n");
    return;

    if (CommandTokens.size() >= 3)
    {
        ShowMessages("incorrect use of the '%s'\n\n",
                     GetCaseSensitiveStringFromCommandToken(CommandTokens.at(0)).c_str());
        CommandMeasureHelp();
        return;
    }

    if (CommandTokens.size() == 2 && !CompareLowerCaseStrings(CommandTokens.at(1), "default"))
    {
        ShowMessages("incorrect use of the '%s'\n\n",
                     GetCaseSensitiveStringFromCommandToken(CommandTokens.at(0)).c_str());
        CommandMeasureHelp();
        return;
    }
    else if (CommandTokens.size() == 2 &&
             CompareLowerCaseStrings(CommandTokens.at(1), "default"))
    {
        DefaultMode = TRUE;
    }

    //
    // Check if debugger is loaded or not
    //
    if (g_DeviceHandle && !DefaultMode)
    {
        ShowMessages(
            "Debugger is loaded and your machine is already in a hypervisor, you "
            "should measure the times before 'load'-ing the debugger, please "
            "'unload' the debugger and use '!measure' again or use '!measure "
            "default' to use hardcoded measurements\n");
        return;
    }

    if (!DefaultMode)
    {
        if (TransparentModeCheckHypervisorPresence(
                &g_CpuidAverage,
                &g_CpuidStandardDeviation,
                &g_CpuidMedian))
        {
            ShowMessages(
                "we detected that there is a hypervisor, on your system, it "
                "leads to wrong measurement results for our transparent-mode, please "
                "make sure that you're not in a hypervisor then measure the result "
                "again; otherwise the transparent-mode will not work but you can use "
                "'!measure default' to use the hardcoded measurements !\n\n");

            return;
        }

        if (TransparentModeCheckRdtscpVmexit(
                &g_RdtscAverage,
                &g_RdtscStandardDeviation,
                &g_RdtscMedian))
        {
            ShowMessages(
                "we detected that there is a hypervisor, on your system, it "
                "leads to wrong measurement results for our transparent-mode, please "
                "make sure that you're not in a hypervisor then measure the result "
                "again; otherwise the transparent-mode will not work but you can use "
                "'!measure default' to use the hardcoded measurements !\n\n");

            return;
        }
    }
    else
    {
        //
        // It's a default mode
        //

        //
        // Default values for cpuid
        //
        g_CpuidAverage           = 0x5f;
        g_CpuidStandardDeviation = 0x10;
        g_CpuidMedian            = 0x5f;

        //
        // Default values for rdtsc/p
        //
        g_RdtscAverage           = 0x16;
        g_RdtscStandardDeviation = 0x5;
        g_RdtscMedian            = 0x16;
    }

    ShowMessages("the measurements were successful\nyou can use the '!hide' command now\n");

    //
    // Indicate that the measurements was successful
    //
    g_TransparentResultsMeasured = TRUE;
}
