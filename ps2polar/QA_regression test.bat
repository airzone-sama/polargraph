%REM run batch files to make all example output command files
call gs_ps2polar_show_origins_cmds
call gs_ps2polar_string_art_cmds
call gs_ps2polar_test_image_cmds
call gs_ps2polar_test_text_cmds
call gs_ps2polar_string_art_C17cmds

%REM  check for differences between canned output files and current output files

fc QA_show_origins_cmds.txt  show_origins_cmds.txt >QA_report.txt
fc QA_string_art_cmds.txt    string_art_cmds.txt   >>QA_report.txt
fc QA_test_image_cmds.txt    test_image_cmds.txt   >>QA_report.txt
fc QA_test_text_cmds.txt     test_text_cmds.txt    >>QA_report.txt
fc QA_string_art_C17cmds.txt    string_art_C17cmds.txt   >>QA_report.txt



