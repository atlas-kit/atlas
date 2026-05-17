function(log_info)
    message(STATUS "${MsgOk}${ARGV}${MsgClr}")
endfunction()

function(log_war)
    message(STATUS "${MsgWar}${ARGV}${MsgClr}")
endfunction()

function(log_err)
    message(SEND_ERROR "${MsgErr}${ARGV}${MsgClr}")
endfunction()

function(log_fatal)
    message(FATAL_ERROR "${MsgErr}${ARGV}${MsgClr}")
endfunction()

function(log_option_enabled OPTION)
    message(STATUS "${MsgBold}Enabled:${MsgClr} ${MsgOk}${OPTION}${MsgClr}")
endfunction()

function(log_option_disabled OPTION)
    message(STATUS "${MsgBold}Disabled:${MsgClr} ${MsgWar}${OPTION}${MsgClr}")
endfunction()
