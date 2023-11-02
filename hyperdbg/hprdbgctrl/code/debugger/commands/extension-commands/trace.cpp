/**
 * @file trace.cpp
 * @author Sina Karvandi (sina@hyperdbg.org)
 * @brief !trace command
 * @details
 * @version 0.7
 * @date 2023-11-02
 *
 * @copyright This project is released under the GNU Public License v3.
 *
 */
#include "pch.h"

/**
 * @brief help of the !trace command
 *
 * @return VOID
 */
VOID
CommandTraceHelp()
{
    ShowMessages("!trace : traces the execution of user-mode/kernel-mode instructions.\n\n");

    ShowMessages("syntax : \t!trace [Mode (string)] [pid ProcessId (hex)] [core CoreId (hex)] [imm IsImmediate (yesno)] "
                 "[sc EnableShortCircuiting (onoff)] [buffer PreAllocatedBuffer (hex)] [script { Script (string) }] "
                 "[condition { Condition (hex) }] [code { Code (hex) }] [output {OutputName (string)}]\n");

    ShowMessages("\n");
    ShowMessages("\t\te.g : !mode u pid 1c0\n");
    ShowMessages("\t\te.g : !mode k pid 1c0\n");
    ShowMessages("\t\te.g : !mode ku pid 1c0\n");
    ShowMessages("\t\te.g : !mode ku core 2 pid 400\n");

    ShowMessages("\n");
    ShowMessages("note: this event applies to the target process; thus, you need to specify the process id\n");
}

/**
 * @brief !trace command handler
 *
 * @param SplittedCommand
 * @param Command
 * @return VOID
 */
VOID
CommandTrace(vector<string> SplittedCommand, string Command)
{
    PDEBUGGER_GENERAL_EVENT_DETAIL     Event                 = NULL;
    PDEBUGGER_GENERAL_ACTION           ActionBreakToDebugger = NULL;
    PDEBUGGER_GENERAL_ACTION           ActionCustomCode      = NULL;
    PDEBUGGER_GENERAL_ACTION           ActionScript          = NULL;
    UINT32                             EventLength;
    UINT32                             ActionBreakToDebuggerLength = 0;
    UINT32                             ActionCustomCodeLength      = 0;
    UINT32                             ActionScriptLength          = 0;
    vector<string>                     SplittedCommandCaseSensitive {Split(Command, ' ')};
    DEBUGGER_EVENT_PARSING_ERROR_CAUSE EventParsingErrorCause;
    BOOLEAN                            SetMode                = FALSE;
    DEBUGGER_EVENT_MODE_TYPE           TargetInterceptionMode = DEBUGGER_EVENT_MODE_TYPE_INVALID;

    //
    // Interpret and fill the general event and action fields
    //
    //
    if (!InterpretGeneralEventAndActionsFields(
            &SplittedCommand,
            &SplittedCommandCaseSensitive,
            TRAP_EXECUTION_SINGLE_INSTRUCTION1,
            &Event,
            &EventLength,
            &ActionBreakToDebugger,
            &ActionBreakToDebuggerLength,
            &ActionCustomCode,
            &ActionCustomCodeLength,
            &ActionScript,
            &ActionScriptLength,
            &EventParsingErrorCause))
    {
        return;
    }

    //
    // Check for size
    //
    if (SplittedCommand.size() > 2)
    {
        ShowMessages("incorrect use of the '!trace'\n");
        CommandTraceHelp();

        FreeEventsAndActionsMemory(Event, ActionBreakToDebugger, ActionCustomCode, ActionScript);
        return;
    }

    //
    // Interpret command specific details (if any)
    //
    for (auto Section : SplittedCommand)
    {
        if (!Section.compare("!trace"))
        {
            continue;
        }
        else if (!Section.compare("u") && !SetMode)
        {
            TargetInterceptionMode = DEBUGGER_EVENT_MODE_TYPE_USER_MODE;
            SetMode                = TRUE;
        }
        else if (!Section.compare("k") && !SetMode)
        {
            TargetInterceptionMode = DEBUGGER_EVENT_MODE_TYPE_KERNEL_MODE;
            SetMode                = TRUE;
        }
        else if ((!Section.compare("uk") || !Section.compare("ku")) && !SetMode)
        {
            TargetInterceptionMode = DEBUGGER_EVENT_MODE_TYPE_USER_MODE_AND_KERNEL_MODE;
            SetMode                = TRUE;
        }
        else
        {
            //
            // Couldn't resolve or unkonwn parameter
            //
            ShowMessages("err, couldn't resolve error at '%s'\n\n",
                         Section.c_str());

            CommandTraceHelp();

            FreeEventsAndActionsMemory(Event, ActionBreakToDebugger, ActionCustomCode, ActionScript);
        }
    }

    //
    // Check if user specified the execution mode or not
    //
    if (!SetMode)
    {
        ShowMessages("please specify the mode(s) that you want to intercept their execution (u, k, ku)\n");

        FreeEventsAndActionsMemory(Event, ActionBreakToDebugger, ActionCustomCode, ActionScript);
        return;
    }

    //
    // Check if user specified the process id or not
    //
    if (Event->ProcessId == DEBUGGER_EVENT_APPLY_TO_ALL_PROCESSES)
    {
        ShowMessages("this event only applies to the selected process(es). please specify "
                     "the 'pid' or the process id of the target process that you want to trap its execution\n");

        FreeEventsAndActionsMemory(Event, ActionBreakToDebugger, ActionCustomCode, ActionScript);
        return;
    }

    //
    // Set the first parameter to the required execution mode
    //
    Event->Options.OptionalParam1 = (UINT64)TargetInterceptionMode;

    //
    // Send the ioctl to the kernel for event registration
    //
    if (!SendEventToKernel(Event, EventLength))
    {
        //
        // There was an error, probably the handle was not initialized
        // we have to free the Action before exit, it is because, we
        // already freed the Event and string buffers
        //

        FreeEventsAndActionsMemory(Event, ActionBreakToDebugger, ActionCustomCode, ActionScript);
        return;
    }

    //
    // Add the event to the kernel
    //
    if (!RegisterActionToEvent(Event,
                               ActionBreakToDebugger,
                               ActionBreakToDebuggerLength,
                               ActionCustomCode,
                               ActionCustomCodeLength,
                               ActionScript,
                               ActionScriptLength))
    {
        //
        // There was an error
        //

        FreeEventsAndActionsMemory(Event, ActionBreakToDebugger, ActionCustomCode, ActionScript);
        return;
    }
}
