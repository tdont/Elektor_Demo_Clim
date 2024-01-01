# Coding rules
* Legacy code do no need to respect the following coding rules
* New code written in legacy sources files (if needed) must respect the philosphy of the source files
* Do not use tools for reformating source code (Ensure git is configured for commit/checkout as is !)
* Do not use tabs but spaces. Indentation is of 4 spaces
* Use cstdint or stdint.h include and only use uint8_t... and so on for standard types
* Use _ in variable and function names (snake case) 
* File names must be camel case (Start with a Uppercase only in case of a class/object)
* For function names, prefix it with the file name : MyWonderfullSuperClass.c => MWSC_my_top_function()
* static function do not need the prefix (and so we now there are static, but this rule is not mandatory)
* Even if there is only one statement, use bracket for if(){}
* Use Allman indentation
* Avoid lines longer than 90 char and do not depass 100 !
* Avoid if possible ternary (and never call function inside a ternary)
* Avoid lambda in C++ (and never create a thread using a lambda !)
* Use C++ Threads and add Cancel and Priority_change.
* Do not use QThread !! (not in the sens of a Thread)
* Do not use QSerial ! 