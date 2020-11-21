BXRI is a very basic project to show redundant C++ includes, as well as duplicate C++ includes.

BXRI is an unfinished project.

BXRI probably currently works better with small projects, and for projects that don't use too many third party dependencies.

BXRI does not parse C++.

For example:

#include "File1.h"

#include "File2.h" // Redundant if File2 is not referenced.

#include "File1.h" // Duplicate include.
